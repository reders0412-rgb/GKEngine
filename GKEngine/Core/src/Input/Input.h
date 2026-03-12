#pragma once

namespace GK {

// 에디터 뷰포트 마우스/키 입력 처리
// C# PointerMoved 등의 이벤트 → P/Invoke → 여기로 전달됨
class Input {
public:
    void mouseMove  (int x, int y);
    void mouseButton(int button, bool pressed, int x, int y);
    void mouseScroll(float delta);
    void key        (int keyCode, bool pressed);

    // 에디터 카메라 상태
    struct EditorCamera {
        float yaw   = -90.f;
        float pitch =  -20.f;
        float distFromTarget = 10.f;
        float targetX = 0, targetY = 0, targetZ = 0;
    };
    const EditorCamera& editorCamera() const { return m_cam; }

private:
    EditorCamera m_cam;
    int  m_lastX = 0, m_lastY = 0;
    bool m_rmb   = false; // right mouse button
    bool m_mmb   = false; // middle mouse button
};

} // namespace GK
