# cmake/CopyMsvcRuntime.cmake
# vswhere로 MSVC CRT DLL 찾아서 dist/ 에 자동 복사
# CMakeLists.txt 에서 -P 스크립트로 호출

if(NOT VSWHERE_EXE OR NOT DIST_DIR)
    message(FATAL_ERROR "VSWHERE_EXE and DIST_DIR must be set")
endif()

execute_process(
    COMMAND "${VSWHERE_EXE}" -latest -property installationPath
    OUTPUT_VARIABLE VS_PATH
    OUTPUT_STRIP_TRAILING_WHITESPACE)

if(NOT VS_PATH)
    message(WARNING "Could not locate Visual Studio via vswhere")
    return()
endif()

file(GLOB REDIST_DIRS "${VS_PATH}/VC/Redist/MSVC/*")
list(SORT REDIST_DIRS ORDER DESCENDING)
list(GET  REDIST_DIRS 0 REDIST_DIR)

set(CRT_DIR "${REDIST_DIR}/x64/Microsoft.VC143.CRT")
if(NOT EXISTS "${CRT_DIR}")
    set(CRT_DIR "${REDIST_DIR}/x64/Microsoft.VC142.CRT")
endif()
if(NOT EXISTS "${CRT_DIR}")
    message(WARNING "CRT directory not found at ${CRT_DIR}")
    return()
endif()

file(GLOB CRT_DLLS "${CRT_DIR}/*.dll")
foreach(DLL IN LISTS CRT_DLLS)
    get_filename_component(DLL_NAME "${DLL}" NAME)
    message(STATUS "  Runtime DLL: ${DLL_NAME}")
    file(COPY "${DLL}" DESTINATION "${DIST_DIR}")
endforeach()

message(STATUS "MSVC runtime DLLs copied from ${CRT_DIR}")
