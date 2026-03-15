# BUGFIX: LNK1181 cannot open input file 'RmlCore.lib'

## 원인

RmlUi 6.0부터 CMake exported target 이름이 변경됨.

| 버전 | target 이름 |
|------|-------------|
| 5.x  | `RmlCore` |
| 6.0+ | `RmlUi::RmlCore` |

`FetchContent`로 빌드한 경우 링커가 `RmlCore.lib`를 직접 찾으려다 실패.  
추가로 `BUILD_SHARED_LIBS`가 기본값(ON)으로 설정되면 `.dll`/`.lib` import lib 경로가 꼬이는 문제도 동반됨.

---

## 수정 내용

### 1. `GKHub/CMakeLists.txt`

#### Before
```cmake
FetchContent_Declare(RmlUi
    GIT_REPOSITORY https://github.com/mikke89/RmlUi.git
    GIT_TAG        6.0 GIT_SHALLOW TRUE)
set(RMLUI_SAMPLES_ENABLED OFF CACHE BOOL "" FORCE)

...

target_link_libraries(GKHub PRIVATE
    RmlCore SDL2::SDL2 SDL2::SDL2main ...)
```

#### After
```cmake
FetchContent_Declare(RmlUi
    GIT_REPOSITORY https://github.com/mikke89/RmlUi.git
    GIT_TAG        6.0 GIT_SHALLOW TRUE)
set(RMLUI_SAMPLES_ENABLED  OFF CACHE BOOL "" FORCE)
set(RMLUI_TESTS_ENABLED    OFF CACHE BOOL "" FORCE)
set(RMLUI_BACKEND_SDL2     OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS      OFF CACHE BOOL "" FORCE)  # static으로 강제

...

target_link_libraries(GKHub PRIVATE
    RmlUi::RmlCore SDL2::SDL2 SDL2::SDL2main ...)  # namespace 추가
```

**핵심 변경:**
- `RmlCore` → `RmlUi::RmlCore` (namespace 형식으로 수정)
- `BUILD_SHARED_LIBS OFF` 추가 → static `.lib` 경로 일관성 확보
- 불필요한 backend/tests 옵션 명시적 OFF

---

### 2. `.github/workflows/build.yml`

- Hub configure step에서 `-DCMAKE_PREFIX_PATH` 제거 (Hub는 Core에 의존하지 않음)
- `-DVCPKG_TARGET_TRIPLET=x64-windows` 명시
- vcpkg install 단계에 `freetype:x64-windows-static` 추가 및 `vcpkg integrate install` 포함

---

## 요약

```
LNK1181: cannot open input file 'RmlCore.lib'
→ target 이름을 RmlUi::RmlCore 로 변경
→ BUILD_SHARED_LIBS OFF 로 static 빌드 강제
```
