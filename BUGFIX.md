# GK Hub 빌드 버그 수정 내역

## 수정된 파일

| 파일 | 변경 내용 |
|------|-----------|
| `GKHub/CMakeLists.txt` | minizip include path 구조 수정, RmlUi include path 추가, `GK_VERSION` define 추가 |
| `GKHub/src/I18n.cpp` | `#include "Common.h"` 추가 |

---

## 버그 1: `minizip/mz.h` No such file or directory

**원인 (핵심):**

minizip-ng 4.x 헤더 구조:
```
_deps/
  minizip-src/      ← minizip_SOURCE_DIR
    mz.h
    mz_strm.h
    mz_zip.h
```

코드에서 `#include <minizip/mz.h>` 형태로 쓰려면
`minizip/` 폴더가 include path 아래에 있어야 함.
→ `minizip_SOURCE_DIR` 자체가 아니라 **그 부모**(`_deps/`)를 잡아야 함.

**수정:**
```cmake
get_filename_component(_mz_parent "${minizip_SOURCE_DIR}" DIRECTORY)

target_include_directories(GKHub PRIVATE
    ${_mz_parent}          # minizip/mz.h 해결
    ${minizip_SOURCE_DIR}  # fallback
)
```

---

## 버그 2: `RmlUi/Core.h` No such file or directory

**수정:**
```cmake
target_include_directories(GKHub PRIVATE
    src
    ${rmlui_SOURCE_DIR}/Include
    ...
)
```

---

## 버그 3: `I18n.cpp` 문법 오류 폭탄

**원인:** `GK_VERSION` 매크로 미정의 (Common.h 미include)

**수정 — I18n.cpp:**
```cpp
#include "I18n.h"
#include "Common.h"   // 추가
#include <iostream>
```

**수정 — CMakeLists.txt:**
```cmake
target_compile_definitions(GKHub PRIVATE
    GK_HUB_VERSION="${GK_VERSION}"
    GK_VERSION="${GK_VERSION}"    # 추가
    ...
)
```
