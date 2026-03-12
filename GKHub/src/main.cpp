#include "HubApp.h"
#ifdef _WIN32
#include <windows.h>
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR lpCmdLine, int) {
    int argc = __argc;
    char** argv = __argv;
    return GK::HubApp::get().run(argc, argv);
}
#else
int main(int argc, char** argv) {
    return GK::HubApp::get().run(argc, argv);
}
#endif
