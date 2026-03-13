#pragma once
union SDL_Event;
namespace Rml { class Context; }
namespace GK {
struct EventHandler {
    // Process one SDL event and route to RmlUi + hub logic
    static void process(const SDL_Event& ev, Rml::Context* ctx);
    // Direct script dispatch (called from C++ side if needed)
    static void dispatchScript(const char* script);
};
}
