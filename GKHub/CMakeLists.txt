cmake_minimum_required(VERSION 3.20)
project(GKHub VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT DEFINED GK_VERSION)
    set(GK_VERSION "1.0")
endif()
if(NOT DEFINED GK_MANIFEST_URL)
    set(GK_MANIFEST_URL "https://raw.githubusercontent.com/reders0412-rgb/GKEngine/main/versions.json")
endif()

include(FetchContent)

FetchContent_Declare(RmlUi
    GIT_REPOSITORY https://github.com/mikke89/RmlUi.git
    GIT_TAG        6.0 GIT_SHALLOW TRUE)
set(RMLUI_SAMPLES_ENABLED OFF CACHE BOOL "" FORCE)

FetchContent_Declare(SDL2
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG        release-2.30.2 GIT_SHALLOW TRUE)

FetchContent_Declare(json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG        v3.11.3 GIT_SHALLOW TRUE)

FetchContent_Declare(CURL
    GIT_REPOSITORY https://github.com/curl/curl.git
    GIT_TAG        curl-8_7_1 GIT_SHALLOW TRUE)
set(BUILD_CURL_EXE    OFF CACHE BOOL "" FORCE)
set(CURL_USE_OPENSSL  OFF CACHE BOOL "" FORCE)
set(CURL_USE_SCHANNEL ON  CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(RmlUi SDL2 json CURL)

add_executable(GKHub WIN32
    src/main.cpp
    src/HubApp.cpp
    src/RmlBackend.cpp
    src/ProjectManager.cpp
    src/VersionManager.cpp
    src/SettingsManager.cpp
    src/I18n.cpp
    src/ThemeManager.cpp
    src/TrayIcon.cpp
    src/SplashScreen.cpp
    src/EventHandler.cpp)

set_target_properties(GKHub PROPERTIES OUTPUT_NAME "GK_Hub")

target_compile_definitions(GKHub PRIVATE
    GK_HUB_VERSION="${GK_VERSION}"
    GK_MANIFEST_URL="${GK_MANIFEST_URL}"
    _CRT_SECURE_NO_WARNINGS NOMINMAX UNICODE _UNICODE)

target_include_directories(GKHub PRIVATE src)

target_link_libraries(GKHub PRIVATE
    RmlCore SDL2::SDL2 SDL2::SDL2main
    nlohmann_json::nlohmann_json CURL::libcurl
    opengl32 dwmapi shell32 ole32 user32)

if(MSVC)
    target_compile_options(GKHub PRIVATE /W4 /MP /O2)
endif()
