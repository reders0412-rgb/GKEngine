# 버그 수정 노트

## C2660: `mz_zip_reader_create` 인자 오류

### 원인

minizip-ng **3.0.9** 에서 `mz_zip_reader_create()` API가 변경됨.
구버전처럼 반환값으로 포인터를 받는 방식이 아니라,
`void**` 를 인자로 넘겨서 내부에서 할당하는 방식으로 바뀐 것.

| 구분 | 코드 |
|------|------|
| ❌ 구버전 (컴파일 에러) | `void* reader = mz_zip_reader_create();` |
| ✅ 신버전 (3.0.9 기준) | `void* reader = NULL;` + `mz_zip_reader_create(&reader);` |

### 수정 파일

- `GKHub/src/VersionManager.cpp` — line 109~114
- `GKHub/src/TemplateManager.cpp` — line 137~142

### 수정 내용

두 파일 모두 동일한 패턴으로 수정:

```cpp
// Before
void* reader = mz_zip_reader_create();

// After
void* reader = NULL;
mz_zip_reader_create(&reader);
```

나머지 open_file, save_all, close, delete 호출은 변경 없음.

### 참고

Warning (C4189, C4100, C4244) 은 미사용 변수/파라미터 관련이라 빌드 실패와 무관.
필요하면 별도로 정리 가능.
