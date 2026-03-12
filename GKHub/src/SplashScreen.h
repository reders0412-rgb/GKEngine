#pragma once
namespace GK {
class SplashScreen {
public:
    void show(const char* imagePath, int ms = 2000);
    void close();
};
}
