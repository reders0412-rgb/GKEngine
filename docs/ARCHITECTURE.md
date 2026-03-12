# GK Suite — 아키텍처 설명서

---

## 전체 구조

```
┌─────────────────────────────────────────────────────────────────┐
│  GK_Hub.exe  (C++ / RmlUi / SDL2)                              │
│                                                                 │
│  ProjectManager   VersionManager   SettingsManager   I18n      │
│       │                │                 │           ThemeManager│
│  projects.json   versions.json    settings.json                 │
│              ↑ GK_MANIFEST_URL fetch                            │
│                                    ↓ 설정 동기화               │
│                          engine_settings.json                   │
└─────────────────────────────┬────────────────────────────────────┘
                              │ 파일시스템 (FileSystemWatcher)
┌─────────────────────────────▼────────────────────────────────────┐
│  GK_Engine.exe  (C# / Avalonia / .NET 8)                        │
│                                                                  │
│  EditorWindow                                                    │
│  ├── MenuBarView          Hierarchy | Scene | Inspector          │
│  ├── ToolbarView          Console   | Project                    │
│  ├── HierarchyPanelView                                          │
│  ├── SceneViewport  ←── NativeControlHost (HWND child)          │
│  ├── InspectorPanelView                                          │
│  └── BottomPanelView (Project + Console)                        │
│                                                                  │
│  EngineService  →  NativeEngine (P/Invoke)                       │
└─────────────────────────────┬────────────────────────────────────┘
                              │ DllImport("GK_Engine_Core.dll")
                              │ extern "C" / __cdecl
┌─────────────────────────────▼────────────────────────────────────┐
│  GK_Engine_Core.dll  (C++ / OpenGL 3.3)                         │
│                                                                  │
│  Engine (싱글턴)                                                  │
│  ├── Scene (game.sce JSON)                                       │
│  │   └── Entity[]  [Transform | Camera | Light | MeshRenderer]  │
│  ├── Renderer  (Win32 OpenGL context, 셰이더, 기즈모)            │
│  ├── Input  (에디터 카메라 orbit/pan/zoom)                       │
│  └── AssetImporter  (Assimp: FBX/GLB/OBJ → ECS)                │
│                                                                  │
│  Public C API: GKEngineAPI.h (GKEngine_Init, GKScene_Load ...)  │
└──────────────────────────────────────────────────────────────────┘
```

---

## 모듈별 설명

### GK Hub (C++)

Unity Hub에 해당하는 프로젝트·버전 관리 도구.

| 클래스 | 파일 | 역할 |
|---|---|---|
| `HubApp` | `HubApp.h/.cpp` | SDL2 윈도우 생성, RmlUi 초기화, 메인 루프 |
| `ProjectManager` | `ProjectManager.h/.cpp` | 프로젝트 생성/열기/삭제, `projects.json` 저장·로드 |
| `VersionManager` | `VersionManager.h/.cpp` | `versions.json` fetch, 다운로드, ZIP 해제, 설치 |
| `SettingsManager` | `SettingsManager.h/.cpp` | `settings.json` 저장·로드, `engine_settings.json` 출력 |
| `I18n` | `I18n.h/.cpp` | 한/영 문자열 테이블, 언어 전환 콜백 |
| `ThemeManager` | `ThemeManager.h/.cpp` | Dark/Light/System 테마, Windows 레지스트리 감지 |
| `TrayIcon` | `TrayIcon.h` | 시스템 트레이 아이콘 |
| `RmlBackend` | `RmlBackend.h` | OpenGL + SDL2 기반 RmlUi 렌더 백엔드 |

**프로젝트 생성 시 폴더 구조:**
```
MyGame/
├── project.gkproj
├── Assets/
│   ├── Scenes/game.sce    ← 기본 씬 (Main Camera, Light, Cube, Plane)
│   ├── Scripts/
│   ├── Textures/
│   ├── Models/
│   ├── Audio/
│   ├── Materials/
│   ├── Prefabs/
│   └── Shaders/
├── ProjectSettings/
│   ├── tags.json
│   └── physics.json
├── Logs/
└── Temp/
```

---

### Hub ↔ Engine 설정 동기화

```
[GK Hub] 설정 변경 (테마/언어)
    │
    ▼
SettingsManager::save()
    │
    ├── settings.json      (Hub 설정 저장)
    └── engine_settings.json  (Engine용 JSON 출력)
              │
              │ FileSystemWatcher (16ms 폴링)
              ▼
[GK Engine] EngineSettings::Load()
    │
    ├── I18nService::SetLanguage()    → 패널 제목 즉시 갱신
    ├── App.RequestedThemeVariant =   → 다크/라이트 즉시 전환
    └── EditorViewModel.RefreshI18n() → 바인딩 갱신
```

**engine_settings.json 예시:**
```json
{
  "language": "ko",
  "theme": "dark",
  "strings": {
    "engine.hierarchy": "하이어라키",
    "engine.inspector": "인스펙터",
    "engine.play": "플레이"
  }
}
```

---

### GK Engine Core (C++ DLL)

**GKEngineAPI.h — C ↔ C# 계약 규칙:**
- 모든 함수: `extern "C"` + `__cdecl` (P/Invoke 호환)
- 문자열: `const char*` (UTF-8)
- 핸들: 불투명 `void*` → C#에서 `IntPtr`
- 에러: `GK_RESULT` 반환값

**핵심 흐름 — 렌더 루프:**
```
C# DispatcherTimer (16ms)
    │
    ▼ P/Invoke
GKEngine_Tick(dt)          → Engine::tick() → 스크립트 Update()
GKEngine_RenderScene()     → Renderer::renderScene()
    │                            │
    │                            ├── drawGrid()
    │                            ├── drawEntity(e) for each entity
    │                            ├── drawSelectionOutline(selected)
    │                            └── drawGizmo(selected)
    │                            └── SwapBuffers(hdc) → HWND child에 표시
    ▼ (Game 탭 활성 시)
GKEngine_RenderGame()      → Renderer::renderGame()
```

**ECS 컴포넌트 구조:**
```cpp
Entity
├── TransformData   { position, rotation, scale }   // 항상 있음
├── CameraData?     { fov, near, far }
├── LightData?      { type, color, intensity }
└── MeshData?       { meshName, material }
```

---

### GK Engine UI (C#)

**P/Invoke 레이어:**
```
NativeEngine.cs          DllImport("GK_Engine_Core.dll")
    │
EngineService.cs         C# 래퍼 싱글턴, DispatcherTimer 렌더 루프
    │
EditorViewModel.cs       ReactiveUI MVVM, SyncFromCore() → Hierarchy
    │
SceneViewport.axaml.cs   NativeControlHost HWND → GKEngine_Init()
                         PointerMoved → GKInput_MouseMove() 등
```

**Inspector 컴포넌트 렌더링:**
```
SelectedObject 변경
    → LoadInspector(item)
        → NativeEngine.GKTransform_GetPosition() → TransformComponent
        → NativeEngine.GKCamera_HasComponent()   → CameraComponent (있을 때만)
        → NativeEngine.GKLight_HasComponent()    → LightComponent  (있을 때만)
        → Components.Clear() + Add()
    → InspectorPanelView (DataTemplate 자동 렌더링)
```

**DetachablePanelView — 패널 분리:**
```
⧉ 버튼 클릭
    → Detach()
        → 부모에서 ContentPresenter 분리
        → 새 Window 생성 (패널 내용 이동)
창 닫힘
    → Redock()
        → 원래 위치로 ContentPresenter 복귀
```

---

## 빌드 순서 (의존성)

```
1. GK_Engine_Core.dll  (C++ CMake)
        ↓ 복사
2. GK_Engine.exe       (C# dotnet publish, DLL 참조)
        ↓ 함께 배포
3. GK_Hub.exe          (C++ CMake, 독립적으로 빌드 가능)
```

---

## 파일 데이터 흐름

```
game.sce (JSON)
    │ GKScene_Load()
    ▼
Scene::load()
    → Entity 목록 생성 (Transform/Camera/Light/Mesh)
    │
    │ GKScene_GetEntityByIndex()
    ▼
EngineService::GetSceneObjects()
    → SceneObjectItem[] (NativeHandle = IntPtr)
    │
    ▼
EditorViewModel::SceneObjects (ObservableCollection)
    │
    ▼
HierarchyPanelView (ListBox 바인딩)
```

---

*© 2024 GeekPiz — MIT License*
