#include "Renderer.h"
#include "../Scene/Scene.h"
#include "../ECS/Entity.h"
#include <windows.h>
#include <GL/gl.h>
#include <GL/glext.h>
#include <cmath>
#include <vector>

// ─── OpenGL function pointers (간소화) ──────────────────────
// 실제 빌드에선 glad 또는 glew를 FetchContent로 가져와 사용 권장
// 여기서는 wglGetProcAddress 패턴만 보여줌
static auto _glGenVertexArrays = (PFNGLGENVERTEXARRAYSPROC)nullptr;
static auto _glBindVertexArray = (PFNGLBINDVERTEXARRAYPROC)nullptr;
static auto _glGenBuffers      = (PFNGLGENBUFFERSPROC)nullptr;
static auto _glBindBuffer      = (PFNGLBINDBUFFERPROC)nullptr;
static auto _glBufferData      = (PFNGLBUFFERDATAPROC)nullptr;
static auto _glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC)nullptr;
static auto _glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC)nullptr;
static auto _glCreateShader    = (PFNGLCREATESHADERPROC)nullptr;
static auto _glShaderSource    = (PFNGLSHADERSOURCEPROC)nullptr;
static auto _glCompileShader   = (PFNGLCOMPILESHADERPROC)nullptr;
static auto _glCreateProgram   = (PFNGLCREATEPROGRAMPROC)nullptr;
static auto _glAttachShader    = (PFNGLATTACHSHADERPROC)nullptr;
static auto _glLinkProgram     = (PFNGLLINKPROGRAMPROC)nullptr;
static auto _glUseProgram      = (PFNGLUSEPROGRAMPROC)nullptr;
static auto _glUniformMatrix4fv= (PFNGLUNIFORMMATRIX4FVPROC)nullptr;
static auto _glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC)nullptr;
static auto _glUniform3f       = (PFNGLUNIFORM3FPROC)nullptr;
static auto _glUniform1i       = (PFNGLUNIFORM1IPROC)nullptr;
static auto _glDeleteVertexArrays = (PFNGLDELETEVERTEXARRAYSPROC)nullptr;
static auto _glDeleteBuffers   = (PFNGLDELETEBUFFERSPROC)nullptr;

#define LOAD_GL(name, type) \
    _ ## name = (type)wglGetProcAddress(#name)

namespace GK {

// ─── Win32 OpenGL context 생성 ───────────────────────────────
bool Renderer::initGL() {
    m_hdc = GetDC((HWND)m_hwnd);
    if (!m_hdc) return false;

    PIXELFORMATDESCRIPTOR pfd{};
    pfd.nSize        = sizeof(pfd);
    pfd.nVersion     = 1;
    pfd.dwFlags      = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType   = PFD_TYPE_RGBA;
    pfd.cColorBits   = 32;
    pfd.cDepthBits   = 24;
    pfd.cStencilBits = 8;

    int fmt = ChoosePixelFormat((HDC)m_hdc, &pfd);
    if (!fmt) return false;
    SetPixelFormat((HDC)m_hdc, fmt, &pfd);

    m_hglrc = wglCreateContext((HDC)m_hdc);
    if (!m_hglrc) return false;
    wglMakeCurrent((HDC)m_hdc, (HGLRC)m_hglrc);

    // OpenGL 3.3 Core — wglCreateContextAttribsARB 사용 권장 (여기선 생략)
    // 실제 빌드 시 glad 초기화:
    // gladLoadGL();

    // GL extension 로드
    LOAD_GL(glGenVertexArrays,     PFNGLGENVERTEXARRAYSPROC);
    LOAD_GL(glBindVertexArray,     PFNGLBINDVERTEXARRAYPROC);
    LOAD_GL(glGenBuffers,          PFNGLGENBUFFERSPROC);
    LOAD_GL(glBindBuffer,          PFNGLBINDBUFFERPROC);
    LOAD_GL(glBufferData,          PFNGLBUFFERDATAPROC);
    LOAD_GL(glVertexAttribPointer, PFNGLVERTEXATTRIBPOINTERPROC);
    LOAD_GL(glEnableVertexAttribArray, PFNGLENABLEVERTEXATTRIBARRAYPROC);
    LOAD_GL(glCreateShader,        PFNGLCREATESHADERPROC);
    LOAD_GL(glShaderSource,        PFNGLSHADERSOURCEPROC);
    LOAD_GL(glCompileShader,       PFNGLCOMPILESHADERPROC);
    LOAD_GL(glCreateProgram,       PFNGLCREATEPROGRAMPROC);
    LOAD_GL(glAttachShader,        PFNGLATTACHSHADERPROC);
    LOAD_GL(glLinkProgram,         PFNGLLINKPROGRAMPROC);
    LOAD_GL(glUseProgram,          PFNGLUSEPROGRAMPROC);
    LOAD_GL(glUniformMatrix4fv,    PFNGLUNIFORMMATRIX4FVPROC);
    LOAD_GL(glGetUniformLocation,  PFNGLGETUNIFORMLOCATIONPROC);
    LOAD_GL(glUniform3f,           PFNGLUNIFORM3FPROC);
    LOAD_GL(glUniform1i,           PFNGLUNIFORM1IPROC);
    LOAD_GL(glDeleteVertexArrays,  PFNGLDELETEVERTEXARRAYSPROC);
    LOAD_GL(glDeleteBuffers,       PFNGLDELETEBUFFERSPROC);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0.1f, 0.1f, 0.14f, 1.f);

    return true;
}

bool Renderer::init(void* hwnd, int w, int h) {
    m_hwnd = hwnd;
    m_w = w; m_h = h;
    if (!initGL()) return false;
    compileShaders();
    buildBuiltinMeshes();
    return true;
}

void Renderer::shutdown() {
    wglMakeCurrent(nullptr, nullptr);
    if (m_hglrc) { wglDeleteContext((HGLRC)m_hglrc); m_hglrc = nullptr; }
    if (m_hwnd && m_hdc) { ReleaseDC((HWND)m_hwnd, (HDC)m_hdc); m_hdc = nullptr; }
}

void Renderer::resize(int w, int h) {
    m_w = w; m_h = h;
    glViewport(0, 0, w, h);
}

// ─── Scene 렌더 (에디터 뷰) ───────────────────────────────────
void Renderer::renderScene(Scene* scene) {
    wglMakeCurrent((HDC)m_hdc, (HGLRC)m_hglrc);
    glViewport(0, 0, m_w, m_h);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (!scene) { SwapBuffers((HDC)m_hdc); return; }

    drawGrid();

    for (int i = 0; i < scene->entityCount(); ++i) {
        auto* e = scene->entityAt(i);
        if (!e->isActive()) continue;
        drawEntity(e);
        if (e->id() == m_selectedId) {
            drawSelectionOutline(e);
            drawGizmo(e);
        }
    }

    SwapBuffers((HDC)m_hdc);
}

// ─── Game 렌더 (게임 카메라 뷰) ──────────────────────────────
void Renderer::renderGame(Scene* scene) {
    wglMakeCurrent((HDC)m_hdc, (HGLRC)m_hglrc);
    glViewport(0, 0, m_w, m_h);
    glClearColor(0.05f, 0.05f, 0.08f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (scene)
        for (int i = 0; i < scene->entityCount(); ++i)
            drawEntity(scene->entityAt(i));

    SwapBuffers((HDC)m_hdc);
}

// ─── 셰이더 컴파일 ───────────────────────────────────────────
bool Renderer::compileShaders() {
    // 간략화 — 실제 파일에서 로드하거나 embedded string 사용
    const char* vs = R"(
#version 330 core
layout(location=0) in vec3 aPos;
layout(location=1) in vec3 aNorm;
uniform mat4 uMVP;
uniform mat4 uModel;
out vec3 vNorm;
out vec3 vFragPos;
void main(){
    vFragPos = vec3(uModel * vec4(aPos,1.0));
    vNorm    = mat3(transpose(inverse(uModel))) * aNorm;
    gl_Position = uMVP * vec4(aPos, 1.0);
})";

    const char* fs = R"(
#version 330 core
in vec3 vNorm;
in vec3 vFragPos;
uniform vec3 uColor;
uniform vec3 uLightDir;
out vec4 FragColor;
void main(){
    float diff = max(dot(normalize(vNorm), normalize(-uLightDir)), 0.0);
    vec3  ambient = uColor * 0.3;
    vec3  diffuse = uColor * diff * 0.7;
    FragColor = vec4(ambient + diffuse, 1.0);
})";

    auto compile = [](const char* src, GLenum type) -> GLuint {
        GLuint s = _glCreateShader(type);
        _glShaderSource(s, 1, &src, nullptr);
        _glCompileShader(s);
        return s;
    };

    GLuint v = compile(vs, GL_VERTEX_SHADER);
    GLuint f = compile(fs, GL_FRAGMENT_SHADER);
    m_shaderProg = _glCreateProgram();
    _glAttachShader(m_shaderProg, v);
    _glAttachShader(m_shaderProg, f);
    _glLinkProgram(m_shaderProg);

    return true;
}

// ─── Built-in 메시 (Cube, Plane) ─────────────────────────────
void Renderer::buildBuiltinMeshes() {
    // 큐브 24 vertices (각 면별 노말)
    static const float cubeVerts[] = {
        // pos            // normal
        -0.5f,-0.5f,-0.5f,  0,0,-1,  0.5f,-0.5f,-0.5f,  0,0,-1,
         0.5f, 0.5f,-0.5f,  0,0,-1, -0.5f, 0.5f,-0.5f,  0,0,-1,
        /* ... 나머지 5면 생략 (실제 구현 시 추가) */
    };
    if (!_glGenVertexArrays) return;
    _glGenVertexArrays(1, &m_cubeVAO);
    _glBindVertexArray(m_cubeVAO);
    _glGenBuffers(1, &m_cubeVBO);
    _glBindBuffer(GL_ARRAY_BUFFER, m_cubeVBO);
    _glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVerts), cubeVerts, GL_STATIC_DRAW);
    _glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,6*sizeof(float),(void*)0);
    _glEnableVertexAttribArray(0);
    _glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,6*sizeof(float),(void*)(3*sizeof(float)));
    _glEnableVertexAttribArray(1);
    _glBindVertexArray(0);
}

// 스텁 — 실제 구현 시 glDrawArrays/glDrawElements 추가
void Renderer::drawGrid()                         { /* TODO: grid lines */ }
void Renderer::drawEntity(Entity*)                { /* TODO: mesh render  */ }
void Renderer::drawGizmo(Entity*)                 { /* TODO: move/rotate/scale gizmo */ }
void Renderer::drawSelectionOutline(Entity*)      { /* TODO: stencil outline */ }

} // namespace GK
