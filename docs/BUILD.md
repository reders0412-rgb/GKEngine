# GK Suite — 빌드 & 설치 설명서

> 버전 **1.0** 고정 — 올리라고 할 때까지 변경 금지

---

## 목차

1. [사전 요구사항 설치](#1-사전-요구사항-설치)
2. [레포 클론](#2-레포-클론)
3. [GK Engine Core 빌드 (C++ DLL)](#3-gk-engine-core-빌드-c-dll)
4. [GK Hub 빌드 (C++)](#4-gk-hub-빌드-c)
5. [GK Engine UI 빌드 (C#)](#5-gk-engine-ui-빌드-c)
6. [배포 패키지 조립](#6-배포-패키지-조립)
7. [GitHub Actions 자동 빌드](#7-github-actions-자동-빌드)
8. [주요 파일 위치 & 버전 변경법](#8-주요-파일-위치--버전-변경법)
9. [트러블슈팅](#9-트러블슈팅)

---

## 1. 사전 요구사항 설치

### 필수 도구

| 도구 | 버전 | 다운로드 |
|---|---|---|
| Visual Studio 2022 | 17.x | https://visualstudio.microsoft.com/ |
| CMake | 3.20 이상 | https://cmake.org/download/ |
| .NET SDK | 8.0 | https://dotnet.microsoft.com/download |
| Git | 최신 | https://git-scm.com/ |

### Visual Studio 2022 설치 시 필수 워크로드
- **C++을 사용한 데스크톱 개발** (MSVC v143 컴파일러 포함)
- **.NET 데스크톱 개발**

### 환경 변수 확인 (PowerShell)
```powershell
cmake --version       # cmake version 3.xx.x
dotnet --version      # 8.0.x
cl                    # VS 개발자 PowerShell에서 실행
```

> 일반 PowerShell에서 `cl`이 안 되면 → **Visual Studio 개발자 PowerShell**을 사용하거나
> `"C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"` 실행

---

## 2. 레포 클론

```powershell
git clone https://github.com/reders0412-rgb/GKEngine.git
cd GKEngine
```

서브모듈이 있다면:
```powershell
git submodule update --init --recursive
```

---

## 3. GK Engine Core 빌드 (C++ DLL)

C++ 렌더러 · ECS 코어. 이것부터 빌드해야 C# UI가 빌드 가능합니다.

```powershell
# 빌드 디렉토리 생성 및 구성
cmake -B build/core GKEngine/Core `
    -G "Visual Studio 17 2022" -A x64 `
    -DCMAKE_BUILD_TYPE=Release

# 빌드 (멀티코어)
cmake --build build/core --config Release -j4
```

**출력 파일:**
```
build/core/Release/GK_Engine_Core.dll   ← 핵심 출력물
build/core/Release/GK_Engine_Core.pdb   ← 디버그 심볼
```

**의존성 자동 다운로드 (FetchContent):**
- nlohmann/json v3.11.3
- Assimp v5.3.1

> 처음 빌드 시 의존성 다운로드로 10~20분 소요될 수 있습니다.
> 이후 빌드는 캐시를 사용하므로 빠릅니다.

---

## 4. GK Hub 빌드 (C++)

프로젝트 관리 및 엔진 버전 다운로드 허브.

```powershell
cmake -B build/hub GKHub `
    -G "Visual Studio 17 2022" -A x64 `
    -DCMAKE_BUILD_TYPE=Release

cmake --build build/hub --config Release -j4
```

**출력 파일:**
```
build/hub/Release/GK_Hub.exe
```

**의존성 자동 다운로드:**
- RmlUi 6.0
- SDL2 2.30
- nlohmann/json 3.11.3
- libcurl 8.7.1 (WinSSL 사용 — OpenSSL 불필요)

**수동으로 추가해야 할 파일:**
```
GKHub/assets/fonts/NotoSans-Regular.ttf    ← Google Fonts에서 다운로드
GKHub/assets/fonts/NotoSansMono-Regular.ttf
GKHub/assets/ui/stb_image.h               ← stb 단일 헤더
```

---

## 5. GK Engine UI 빌드 (C#)

Avalonia 기반 편집기 UI. Core DLL이 반드시 먼저 빌드되어야 합니다.

```powershell
# DLL을 UI 폴더에 먼저 복사
Copy-Item build/core/Release/GK_Engine_Core.dll GKEngine/UI/

# NuGet 패키지 복원
dotnet restore GKEngine/UI/GKEngine.UI.csproj

# Self-contained 아닌 방식으로 퍼블리시 (런타임 별도 설치 필요)
dotnet publish GKEngine/UI/GKEngine.UI.csproj `
    -c Release `
    -r win-x64 `
    --self-contained false `
    -o dist/GKEngineUI `
    /p:Version=1.0
```

**Self-contained 배포 (런타임 포함, 파일 크기 큰 대신 .NET 설치 불필요):**
```powershell
dotnet publish GKEngine/UI/GKEngine.UI.csproj `
    -c Release -r win-x64 --self-contained true `
    -p:PublishSingleFile=true `
    -o dist/GKEngineUI_sc
```

**출력 파일:**
```
dist/GKEngineUI/
├── GK_Engine.exe
├── GK_Engine_Core.dll    ← Core DLL 같이 배포
├── Avalonia.dll
└── ...
```

---

## 6. 배포 패키지 조립

최종 배포 구조:

```
GKSuite-1.0-Windows-x64/
├── GK_Hub.exe
├── GK_Hub.pdb             (선택)
├── assets/
│   ├── ui/
│   │   ├── index.rml
│   │   ├── style.rcss
│   │   ├── theme-dark.rcss
│   │   └── theme-light.rcss
│   ├── fonts/
│   │   └── NotoSans-Regular.ttf
│   └── logo.png
├── versions.json
└── Editors/
    └── 1.0/
        ├── GK_Engine.exe
        ├── GK_Engine_Core.dll
        └── ...
```

PowerShell로 조립:
```powershell
$pkg = "dist/GKSuite-1.0"
New-Item -Force -ItemType Directory $pkg, "$pkg/Editors/1.0"

# Hub
Copy-Item build/hub/Release/GK_Hub.exe $pkg/
Copy-Item -Recurse GKHub/assets $pkg/assets
Copy-Item versions.json $pkg/

# MSVC 런타임 DLL (CI 환경)
$vs  = & "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe" -latest -property installationPath
$crt = (Get-ChildItem "$vs\VC\Redist\MSVC" | Sort Name -Desc | Select -First 1).FullName
Copy-Item "$crt\x64\Microsoft.VC143.CRT\*.dll" $pkg/

# Engine
Copy-Item -Recurse dist/GKEngineUI/* "$pkg/Editors/1.0/"

# ZIP
Compress-Archive "$pkg/*" "GKSuite-1.0-Windows-x64.zip" -Force
```

---

## 7. GitHub Actions 자동 빌드

`.github/workflows/build.yml` 에 정의된 CI 파이프라인:

```
push to main
    │
    ├─ build-hub         → CMake + MSVC → GK_Hub.exe
    │
    ├─ build-engine-core → CMake + MSVC → GK_Engine_Core.dll
    │
    ├─ build-engine-ui   → dotnet publish → GK_Engine.exe
    │   (build-engine-core 완료 후 실행, DLL 자동 복사)
    │
    └─ package           → ZIP → Release artifact
         (main 브랜치 push 시에만 실행)
```

### Actions에서 Artifact 다운로드

1. GitHub → `Actions` 탭 → 최신 성공 빌드 클릭
2. 하단 **Artifacts** 섹션에서 `GKSuite-Release-1.0` 다운로드

### 버전 변경 시 (Actions에서)

```yaml
# .github/workflows/build.yml 상단
env:
  GK_VERSION: "1.0"   ← 이것만 변경
```

---

## 8. 주요 파일 위치 & 버전 변경법

### 버전 관련

| 변경 항목 | 파일 | 위치 |
|---|---|---|
| CI 빌드 버전 (가장 중요) | `.github/workflows/build.yml` | `env: GK_VERSION:` |
| Hub C++ 버전 상수 | `GKHub/src/Common.h` | `#define GK_HUB_VERSION` |
| Hub CMake 버전 | `GKHub/CMakeLists.txt` | `project(GKHub VERSION ...)` |
| Engine C++ 버전 | `GKEngine/Core/CMakeLists.txt` | `project(GKEngineCore VERSION ...)` |
| C# UI 버전 | `dotnet publish /p:Version=...` | CLI 파라미터 또는 `.csproj` |

### Manifest URL (versions.json 위치)

| 파일 | 위치 |
|---|---|
| `GKHub/CMakeLists.txt` | `target_compile_definitions` 안의 `GK_MANIFEST_URL` |
| `GKHub/src/Common.h` | `#define GK_MANIFEST_URL` (fallback) |

### C++ ↔ C# 인터페이스

| 파일 | 역할 |
|---|---|
| `GKEngine/Core/include/GKEngineAPI.h` | C API 공개 헤더 (계약서) |
| `GKEngine/Core/src/GKEngineAPI.cpp` | C API 구현 |
| `GKEngine/UI/Interop/NativeEngine.cs` | C# P/Invoke 선언 |
| `GKEngine/UI/Services/EngineService.cs` | C# 래퍼 서비스 |

---

## 9. 트러블슈팅

### CMake: FetchContent 다운로드 실패
```
해결: 방화벽/VPN 확인, 또는 수동으로 deps 소스를 다운받아
      build/_deps/ 에 배치 후 오프라인 빌드 사용
```

### MSVC: LNK2019 링크 에러
```
해결: 모든 .cpp 파일이 CMakeLists.txt의 소스 목록에 있는지 확인
     add_executable() 또는 add_library() 에 파일 추가
```

### dotnet: P/Invoke DllNotFoundException
```
해결: GK_Engine_Core.dll이 GK_Engine.exe와 같은 디렉토리에 있는지 확인
     CI에서는 build-engine-core artifact를 UI 빌드 전에 다운로드
```

### Avalonia: NativeControlHost HWND null
```
해결: HandleCreated 이벤트에서 HWND를 얻어야 함
     생성자에서 GKEngine_Init() 호출 금지
     SceneViewport.axaml.cs의 OnHandleCreated() 확인
```

### RmlUi: 폰트 로드 실패
```
해결: NotoSans-Regular.ttf를 assets/fonts/ 에 배치
     Google Fonts: https://fonts.google.com/noto/specimen/Noto+Sans
```

### libcurl SSL 에러 (Windows)
```
해결: CMakeLists.txt에 CURL_USE_SCHANNEL=ON 확인
     Windows 내장 TLS를 사용하므로 OpenSSL 불필요
```

---

*© 2024 GeekPiz — MIT License*
