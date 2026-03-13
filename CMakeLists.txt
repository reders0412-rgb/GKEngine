cmake_minimum_required(VERSION 3.20)
project(GKSuite VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ===================================================================
#  GK Suite - 루트 빌드 파일
#  cmake -B build -G "Visual Studio 17 2022" -A x64
#  cmake --build build --config Release
#  -> dist/ 에 모든 결과물 자동 조립
#
#  ★ 버전 변경:     GK_VERSION 한 줄
#  ★ Manifest URL: GK_MANIFEST_URL 한 줄
# ===================================================================

set(GK_VERSION      "1.0")
set(GK_MANIFEST_URL "https://raw.githubusercontent.com/reders0412-rgb/GKEngine/main/versions.json")
set(GK_DIST_DIR     "${CMAKE_SOURCE_DIR}/dist")

# .NET SDK 찾기
# ★ Fix: $ENV{ProgramFiles(x86)} 는 CMake 에서 '(' ')' 가 변수 이름에
#         허용되지 않으므로 하드코딩 경로로 커버합니다.
set(_dotnet_hints
    "$ENV{DOTNET_ROOT}"
    "$ENV{ProgramFiles}/dotnet"
    "C:/Program Files/dotnet"
    "C:/Program Files (x86)/dotnet"
)
find_program(DOTNET_EXE dotnet REQUIRED HINTS ${_dotnet_hints})
message(STATUS "[GKSuite] dotnet: ${DOTNET_EXE}")

# ===================================================================
#  STEP 1: GK Engine Core (C++ DLL)
# ===================================================================
add_subdirectory(GKEngine/Core)

# DLL -> GKEngine/UI/ 에 복사 (C# 빌드 전에 필요)
# DLL -> dist/ 에도 복사
add_custom_command(TARGET GKEngineCore POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${GK_DIST_DIR}"
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different
        "$<TARGET_FILE:GKEngineCore>"
        "${CMAKE_SOURCE_DIR}/GKEngine/UI/GK_Engine_Core.dll"
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different
        "$<TARGET_FILE:GKEngineCore>"
        "${GK_DIST_DIR}/GK_Engine_Core.dll"
    COMMENT "[1/4] GK_Engine_Core.dll -> UI/ + dist/"
)

# ===================================================================
#  STEP 2: GK Engine UI (C# / Avalonia) - Core 빌드 후 자동 실행
# ===================================================================
set(CS_DIR "${CMAKE_SOURCE_DIR}/GKEngine/UI")
set(CS_PUBLISH_OUT "${GK_DIST_DIR}/Editors/${GK_VERSION}")

add_custom_target(GKEngineUI ALL
    DEPENDS GKEngineCore
    COMMAND "${CMAKE_COMMAND}" -E echo "[2/4] Building C# Engine UI..."
    COMMAND "${DOTNET_EXE}" restore "${CS_DIR}/GKEngine.UI.csproj"
    COMMAND "${DOTNET_EXE}" publish "${CS_DIR}/GKEngine.UI.csproj"
        --configuration Release
        --runtime win-x64
        --self-contained false
        --output "${CS_PUBLISH_OUT}"
        /p:Version=${GK_VERSION}.0
    COMMAND "${CMAKE_COMMAND}" -E echo "[2/4] GK_Engine.exe -> dist/Editors/${GK_VERSION}/"
    WORKING_DIRECTORY "${CS_DIR}"
    COMMENT "Publishing GK Engine UI"
)

# ===================================================================
#  STEP 3: GK Hub (C++)
# ===================================================================
add_subdirectory(GKHub)

add_custom_command(TARGET GKHub POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${GK_DIST_DIR}"
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different
        "$<TARGET_FILE:GKHub>"
        "${GK_DIST_DIR}/GK_Hub.exe"
    COMMENT "[3/4] GK_Hub.exe -> dist/"
)

# ===================================================================
#  STEP 4: dist/ 최종 조립 (assets, versions.json)
# ===================================================================
add_custom_target(GKSuiteDist ALL
    DEPENDS GKHub GKEngineUI
    COMMAND "${CMAKE_COMMAND}" -E echo "[4/4] Assembling dist/..."
    COMMAND "${CMAKE_COMMAND}" -E copy_directory
        "${CMAKE_SOURCE_DIR}/GKHub/assets"
        "${GK_DIST_DIR}/assets"
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different
        "${CMAKE_SOURCE_DIR}/versions.json"
        "${GK_DIST_DIR}/versions.json"
    COMMAND "${CMAKE_COMMAND}" -E echo
        "[GKSuite] Build complete! -> ${GK_DIST_DIR}"
    COMMENT "Assembling final dist/"
)

# ===================================================================
#  STEP 5: MSVC 런타임 DLL 자동 복사
# ===================================================================
if(MSVC)
    # vswhere 는 거의 항상 ProgramFiles(x86)/Microsoft Visual Studio/Installer 에 있음
    # CMake ENV{} 파싱 버그를 피하기 위해 하드코딩 경로 + PATH 검색 조합 사용
    find_program(VSWHERE_EXE vswhere
        HINTS
            "C:/Program Files (x86)/Microsoft Visual Studio/Installer"
            "C:/Program Files/Microsoft Visual Studio/Installer"
        PATHS ENV PATH)
    if(VSWHERE_EXE)
        add_custom_target(CopyMsvcRuntime ALL
            DEPENDS GKHub
            COMMAND "${CMAKE_COMMAND}"
                -DVSWHERE_EXE="${VSWHERE_EXE}"
                -DDIST_DIR="${GK_DIST_DIR}"
                -P "${CMAKE_SOURCE_DIR}/cmake/CopyMsvcRuntime.cmake"
            COMMENT "Copying MSVC CRT DLLs to dist/")
        add_dependencies(GKSuiteDist CopyMsvcRuntime)
    else()
        message(WARNING "[GKSuite] vswhere.exe not found - MSVC runtime DLLs NOT copied.")
        message(WARNING "          Run vcredist_x64.exe on target machine, or install manually.")
    endif()
endif()

message(STATUS "")
message(STATUS "=== GK Suite Build ===")
message(STATUS "  Version     : ${GK_VERSION}")
message(STATUS "  Output      : ${GK_DIST_DIR}")
message(STATUS "  .NET SDK    : ${DOTNET_EXE}")
message(STATUS "")
message(STATUS "  Run:")
message(STATUS "    cmake --build build --config Release")
message(STATUS "  Result: dist/ contains everything")
message(STATUS "")
