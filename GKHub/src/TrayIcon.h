#pragma once
#include <functional>
namespace GK {
class TrayIcon {
public:
    void create(const char* tooltip, std::function<void()> onShow);
    void destroy();
    void showNotification(const char* title, const char* msg);
};
}
