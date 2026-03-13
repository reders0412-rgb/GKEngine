# GK Hub 빌드 버그 수정 내역

## 고친 버그 3개

---

### 1. `RmlUi/Core.h`: No such file or directory

**원인:** `FetchContent_MakeAvailable(RmlUi)`로 RmlUi를 다운받아도,
`target_include_directories`에 RmlUi의 Include 경로를 명시하지 않으면
MSVC가 헤더를 못 찾음.

**수정 파일:** `GKHub/CMakeLists.txt`

```cmake
# 전
target_include_directories(GKHub PRIVATE src)

# 후
target_include_directories(GKHub PRIVATE
    src
    ${rmlui_SOURCE_DIR}/Include    # RmlUi/Core.h 등
    ${_mz_inc}                     # minizip/mz.h 등
)
```

---

### 2. `minizip/mz.h`: No such file or directory

**원인:** `minizip-ng`은 FetchContent로 가져와도 INTERFACE_INCLUDE_DIRECTORIES가
자동으로 전파되지 않는 경우가 있음. include path를 직접 잡아줘야 함.

**수정 파일:** `GKHub/CMakeLists.txt`

```cmake
# FetchContent_MakeAvailable 바로 아래에 추가
get_target_property(_mz_inc minizip INTERFACE_INCLUDE_DIRECTORIES)
if(NOT _mz_inc)
    set(_mz_inc "${minizip_SOURCE_DIR}")
endif()

# target_include_directories에 ${_mz_inc} 추가 (위 항목과 동일)
```

---

### 3. `I18n.cpp` 70번째 줄 문법 오류 (C2059, C2146, C2447 등 대량 발생)

**원인:** `I18n.cpp`의 `loadEnglish()` 안에서 `GK_VERSION` 매크로를 사용하는데,
`I18n.cpp`가 `Common.h`를 include하지 않아서 매크로가 미정의 상태였음.
CMake에서는 `GK_HUB_VERSION`으로 define하고 있었고, `GK_VERSION`은 누락됨.

두 가지 동시에 수정:

**수정 파일 1:** `GKHub/src/I18n.cpp`

```cpp
// 전
#include "I18n.h"
#include <iostream>

// 후
#include "I18n.h"
#include "Common.h"   // ← GK_VERSION 매크로 제공
#include <iostream>
```

**수정 파일 2:** `GKHub/CMakeLists.txt`

```cmake
# 전
target_compile_definitions(GKHub PRIVATE
    GK_HUB_VERSION="${GK_VERSION}"
    ...

# 후
target_compile_definitions(GKHub PRIVATE
    GK_HUB_VERSION="${GK_VERSION}"
    GK_VERSION="${GK_VERSION}"     # ← I18n.cpp가 직접 참조하는 매크로
    ...
```

---

## 수정된 파일 목록

| 파일 | 변경 내용 |
|------|-----------|
| `GKHub/CMakeLists.txt` | RmlUi include path 추가, minizip include path 자동 감지, `GK_VERSION` define 추가 |
| `GKHub/src/I18n.cpp` | `#include "Common.h"` 추가 |

## 적용 방법

수정된 두 파일을 기존 프로젝트에 덮어씌우면 됨.
GitHub Actions에서 새로 push하면 빌드 통과할 것.
