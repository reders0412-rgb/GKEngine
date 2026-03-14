// ─────────────────────────────────────────────────────────────
//  GK Hub RmlBackend — SDL2 + OpenGL3 backend for RmlUi
//  Uses RmlUi's built-in SDL/GL3 backend helpers.
// ─────────────────────────────────────────────────────────────
#include "RmlBackend.h"
#include <RmlUi/Core.h>
// RmlUi ships sample backends; we use the GL3 + SDL2 one.
// If the full backend sources are vendored, include them here.
// For now, provide a minimal stub that compiles cleanly.
// Replace with the real RmlUi SDL/GL3 backend for production.
// SDL2 반드시 먼저: WINGDIAPI / APIENTRY 매크로를 SDL2가 정의해야 gl.h가 정상 파싱됨
#include <SDL.h>
#include <SDL_opengl.h>

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
