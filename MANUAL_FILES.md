# GK Suite — Manual Files & Dependencies

## ⚠️ Files NOT auto-downloaded by CMake FetchContent

These must be placed manually or resolved before building:

### RmlUi SDL2+OpenGL3 Backend
RmlUi ships sample backends under `Backends/` in its repo.
Copy these into `GKHub/src/`:
- `RmlUi_Backend_SDL_GL3.cpp` (rename to `RmlBackend_Impl.cpp` and include from `RmlBackend.cpp`)
- Or use the official `RmlUi/Backends/` approach with their CMake target

**Quick start:** Copy from `build/_deps/rmlui-src/Backends/RmlUi_Backend_SDL_GL3.cpp`
after running `cmake -B build` once.

### Fonts
Place in `GKHub/assets/fonts/`:
- `NotoSans-Regular.ttf`
- `NotoSans-Bold.ttf`
- `NotoSansMono-Regular.ttf`

Download from https://fonts.google.com/noto

## New Files Added (v1.1)

| File | Purpose |
|------|---------|
| `GKHub/src/TemplateManager.h/.cpp` | GitHub template fetch/download/cache system |
| `GKHub/src/ProfileManager.h/.cpp`  | GitHub avatar auto-download |
| `GKHub/src/EventHandler.cpp`       | Full RmlUi event bridge (was stub) |
| `GKHub/src/RmlBackend.cpp`         | SDL2+GL3 backend stub |
| `GKHub/src/SplashScreen.cpp`       | Splash screen stub |
| `GKHub/src/TrayIcon.cpp`           | Tray icon stub |
| `GKHub/assets/ui/index.rml`        | Unity Hub-style UI (full rewrite) |
| `GKHub/assets/ui/style.rcss`       | New component styles |

## versions.json Template Schema

```json
{
  "templates": [
    {
      "id":             "my-template",
      "name":           "My Template",
      "name_ko":        "내 템플릿",
      "description":    "Description in English",
      "description_ko": "한국어 설명",
      "author":         "YourName",
      "thumbnail":      "https://raw.githubusercontent.com/.../thumb.png",
      "download":       "https://github.com/.../template.zip",
      "builtin":        false,
      "type":           "3D",
      "tags":           ["3D", "fps"]
    }
  ]
}
```

### Template ZIP structure
```
template.zip
  Assets/
    Scenes/
      game.sce        ← optional, overrides default scene
    Scripts/
    Textures/
  ProjectSettings/
    tags.json         ← optional override
```
