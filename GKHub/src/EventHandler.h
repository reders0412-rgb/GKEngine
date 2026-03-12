#pragma once
union SDL_Event;
namespace Rml { class Context; }
namespace GK {
struct EventHandler {
    static void process(const SDL_Event& ev, Rml::Context* ctx);
};
}
