#include "TrayIcon.h"
namespace GK {
void TrayIcon::create(const char*, std::function<void()>) {}
void TrayIcon::destroy() {}
void TrayIcon::showNotification(const char*, const char*) {}
}
