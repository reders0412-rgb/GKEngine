#include "Input.h"
#include <cmath>

namespace GK {

void Input::mouseMove(int x, int y) {
    int dx = x - m_lastX;
    int dy = y - m_lastY;
    m_lastX = x; m_lastY = y;

    if (m_rmb) {
        // Alt + RMB = 에디터 카메라 orbit
        m_cam.yaw   += dx * 0.3f;
        m_cam.pitch += dy * 0.3f;
        if (m_cam.pitch >  89.f) m_cam.pitch =  89.f;
        if (m_cam.pitch < -89.f) m_cam.pitch = -89.f;
    }
    if (m_mmb) {
        // MMB = pan
        m_cam.targetX -= dx * 0.01f;
        m_cam.targetY += dy * 0.01f;
    }
}

void Input::mouseButton(int button, bool pressed, int x, int y) {
    m_lastX = x; m_lastY = y;
    if (button == 1) m_rmb = pressed;
    if (button == 2) m_mmb = pressed;
}

void Input::mouseScroll(float delta) {
    m_cam.distFromTarget -= delta * 0.5f;
    if (m_cam.distFromTarget < 0.5f) m_cam.distFromTarget = 0.5f;
}

void Input::key(int keyCode, bool pressed) {
    // W/A/S/D 플라이 카메라 — Play 모드 때 사용
    (void)keyCode; (void)pressed;
}

} // namespace GK
