cmake_minimum_required(VERSION 3.20)
project(GKEngineCore VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 루트에서 GK_VERSION 전달받음 (없으면 fallback)
if(NOT DEFINED GK_VERSION)
    set(GK_VERSION "1.0")
endif()

include(FetchContent)

FetchContent_Declare(nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG        v3.11.3 GIT_SHALLOW TRUE)

FetchContent_Declare(assimp
    GIT_REPOSITORY https://github.com/assimp/assimp.git
    GIT_TAG        v5.3.1 GIT_SHALLOW TRUE)
set(BUILD_SHARED_LIBS   OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_TESTS  OFF CACHE BOOL "" FORCE)
set(ASSIMP_INSTALL      OFF CACHE BOOL "" FORCE)
set(ASSIMP_NO_EXPORT    ON  CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_FBX_IMPORTER ON CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_GLTF_IMPORTER ON CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_OBJ_IMPORTER ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(nlohmann_json assimp)

add_library(GKEngineCore SHARED
    src/GKEngineAPI.cpp
    src/Engine.cpp
    src/Renderer/Renderer.cpp
    src/Scene/Scene.cpp
    src/ECS/Entity.cpp
    src/Input/Input.cpp
)

set_target_properties(GKEngineCore PROPERTIES
    OUTPUT_NAME "GK_Engine_Core")

target_compile_definitions(GKEngineCore PRIVATE
    GKCORE_EXPORTS
    GK_ENGINE_VERSION="${GK_VERSION}"
    _CRT_SECURE_NO_WARNINGS
    NOMINMAX)

target_include_directories(GKEngineCore PUBLIC include src)

target_link_libraries(GKEngineCore PRIVATE
    nlohmann_json::nlohmann_json
    assimp::assimp
    opengl32 gdi32 user32 dwmapi)

if(MSVC)
    target_compile_options(GKEngineCore PRIVATE /W4 /MP /O2)
endif()
