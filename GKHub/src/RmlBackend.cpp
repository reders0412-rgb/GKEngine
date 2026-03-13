// ─────────────────────────────────────────────────────────────
//  GK Hub RmlBackend — SDL2 + OpenGL3 backend for RmlUi
//  Uses RmlUi's built-in SDL/GL3 backend helpers.
// ─────────────────────────────────────────────────────────────
#include "RmlBackend.h"
// Windows 헤더를 반드시 먼저 include해야 WINGDIAPI/APIENTRY가 정의됨
// SDL이 내부에서 windows.h를 불완전하게 땡겨오면 gl.h가 폭발함
#ifdef _WIN32
#  define WIN32_LEAN_AND_MEAN
#  define NOMINMAX
#  include <windows.h>
#endif
#include <GL/gl.h>
#include <SDL.h>
#include <RmlUi/Core.h>

namespace GK {

void RmlBackend::init(SDL_Window*, void*) {
    // TODO: initialize RmlUi GL3 render interface + SDL2 system interface
}

void RmlBackend::processEvent(const SDL_Event&) {
    // SDL events forwarded to RmlUi context handled in EventHandler::process
}

void RmlBackend::beginFrame() {
    glClearColor(0.05f, 0.05f, 0.08f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

void RmlBackend::endFrame(SDL_Window* w) {
    SDL_GL_SwapWindow(w);
}

void RmlBackend::shutdown() {}

} // namespace GK
