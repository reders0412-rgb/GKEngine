# 버그픽스: gl.h 파싱 오류 & minizip include 경로 누락

## 에러 1 — `gl.h` WINGDIAPI/APIENTRY 파싱 폭발

### 원인
`RmlBackend.cpp`에서 include 순서가 잘못됨:
```cpp
// 🔴 잘못된 순서
#include <SDL.h>
#include <GL/gl.h>
```
SDL이 내부적으로 `<windows.h>`를 **불완전하게** include함.  
그 상태에서 `<GL/gl.h>`를 열면 `WINGDIAPI`, `APIENTRY` 매크로가 정의되지 않아
100개 넘는 파싱 에러가 폭발함.

### 수정 (`GKHub/src/RmlBackend.cpp`)
```cpp
// ✅ 올바른 순서: windows.h → gl.h → SDL.h
#ifdef _WIN32
#  define WIN32_LEAN_AND_MEAN
#  define NOMINMAX
#  include <windows.h>
#endif
#include <GL/gl.h>
#include <SDL.h>
#include <RmlUi/Core.h>
```
`windows.h`를 먼저 include해서 `WINGDIAPI`/`APIENTRY`를 완전히 정의한 뒤 `gl.h`를 include해야 함.

---

## 에러 2 — `minizip/mz.h`: No such file or directory

### 원인
`FetchContent`로 minizip-ng를 가져왔지만 `INTERFACE_INCLUDE_DIRECTORIES`가  
빈 값으로 반환되는 경우 SOURCE_DIR을 폴백으로 쓰는 코드가 있었으나,  
`list(APPEND)` 가 누락되어 SOURCE_DIR이 실제로 include 경로에 추가되지 않음.

minizip-ng 4.x는 헤더를 소스 루트에 직접 둠:
```
minizip_SOURCE_DIR/
  minizip/
    mz.h
    mz_zip.h
    ...
```

### 수정 (`GKHub/CMakeLists.txt`)
```cmake
get_target_property(_mz_inc minizip INTERFACE_INCLUDE_DIRECTORIES)
if(NOT _mz_inc)
    set(_mz_inc "")
endif()
# SOURCE_DIR을 명시적으로 추가 (4.x 헤더 구조 대응)
list(APPEND _mz_inc "${minizip_SOURCE_DIR}")
```
그리고 `target_include_directories`에서 `${_mz_inc}`가 들어가므로  
`minizip/mz.h` 경로가 정상적으로 잡힘.

---

## 수정된 파일 목록

| 파일 | 변경 내용 |
|------|-----------|
| `GKHub/src/RmlBackend.cpp` | include 순서 변경 (windows.h → gl.h → SDL.h) |
| `GKHub/CMakeLists.txt` | minizip SOURCE_DIR을 include 경로에 명시적으로 추가 |
