#pragma once
struct SDL_Window; union SDL_Event;
namespace Rml { class Context; }

namespace GK {
struct RmlBackend {
    static void init(SDL_Window* w, void* glCtx);
    static void processEvent(const SDL_Event& ev);
    static void beginFrame();
    static void endFrame(SDL_Window* w);
    static void shutdown();
};
}
