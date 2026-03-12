# 직접 넣어야 할 파일 목록

> **빌드 전에** 아래 파일들을 지정 경로에 직접 복사해두면 됨.  
> 빌드 결과물(dist/)은 건드릴 필요 없고, **프로젝트 폴더 안에만** 넣으면 됨.

---

## 1. Noto Sans 폰트 (필수)

Hub UI 텍스트 렌더링에 필요. 없으면 글자가 안 나옴.

**다운로드:**
- https://fonts.google.com/noto/specimen/Noto+Sans
- "Download family" 버튼 클릭 → ZIP 압축 해제

**넣을 위치:**
```
GKHub/assets/fonts/NotoSans-Regular.ttf
GKHub/assets/fonts/NotoSans-Bold.ttf
GKHub/assets/fonts/NotoSansKR-Regular.ttf      ← 한국어 필요 시
GKHub/assets/fonts/NotoSansMono-Regular.ttf    ← 모노스페이스 (코드/경로 표시)
```

---

## 2. stb_image.h (필수 - Hub 이미지 로딩)

단일 헤더 파일. 로고·아이콘 PNG 로딩에 사용.

**다운로드:**
- https://github.com/nothings/stb/blob/master/stb_image.h
- Raw 버튼 → 우클릭 → 다른 이름으로 저장

**넣을 위치:**
```
GKHub/src/stb_image.h
```

---

## 3. 로고 이미지 (선택 - 이미 있으면 OK)

Hub About 화면과 스플래시 스크린에 표시됨.

**요구사항:** PNG, 권장 512×512 px

**넣을 위치:**
```
GKHub/assets/logo.png      ← 이미 있음 (교체 원하면 덮어쓰기)
```

---

## 최종 프로젝트 폴더 구조 (빌드 전)

```
GKEngine/  (레포 루트)
├── CMakeLists.txt
├── cmake/
│   └── CopyMsvcRuntime.cmake
├── GKHub/
│   ├── CMakeLists.txt
│   ├── assets/
│   │   ├── logo.png
│   │   ├── fonts/
│   │   │   ├── NotoSans-Regular.ttf       ★ 직접 넣기
│   │   │   ├── NotoSans-Bold.ttf          ★ 직접 넣기
│   │   │   ├── NotoSansKR-Regular.ttf     ★ 직접 넣기 (한국어)
│   │   │   └── NotoSansMono-Regular.ttf   ★ 직접 넣기
│   │   └── ui/
│   │       ├── index.rml
│   │       ├── style.rcss
│   │       ├── theme-dark.rcss
│   │       └── theme-light.rcss
│   └── src/
│       ├── stb_image.h                    ★ 직접 넣기
│       └── ... (나머지 소스)
├── GKEngine/
│   ├── Core/   (C++ DLL 소스)
│   └── UI/     (C# Avalonia 소스)
├── versions.json
└── .github/workflows/build.yml
```

---

## 빌드 명령 (이게 전부)

```powershell
# 1회 configure
cmake -B build -G "Visual Studio 17 2022" -A x64

# 빌드 (Hub + Core DLL + Engine UI 전부)
cmake --build build --config Release

# 결과물은 dist/ 에 자동 조립됨
```

**dist/ 최종 구조:**
```
dist/
├── GK_Hub.exe
├── GK_Engine_Core.dll
├── vcruntime140.dll        ← MSVC 런타임 자동 복사
├── assets/
│   ├── fonts/
│   └── ui/
├── versions.json
└── Editors/
    └── 1.0/
        ├── GK_Engine.exe
        ├── GK_Engine_Core.dll
        └── (Avalonia DLL들)
```

> `GK_Hub.exe` 실행하면 됨. 나머지는 Hub가 자동으로 관리.
