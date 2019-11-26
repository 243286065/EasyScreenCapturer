#include "EasyScreenCapturer.h"
#include <thread>

#if defined(OS_WIN)
#pragma comment("EasyScreenCapture.lib")
#endif

int main() {
    media::EasyScreenCapturer::GetInstance()->CaptureFullScreenAsBmp("test0.bmp");
    media::EasyScreenCapturer::GetInstance()->CaptureScreenAsBmp("test.bmp", 0, 0, 1366, 768);
    media::EasyScreenCapturer::GetInstance()->CaptureScreenAsBmp("test1.bmp", 100, 100, 1266, 668);
    int i = 0;
    while (i++ < 1000) {
        std::string filename = std::string("img/test_") + std::to_string(i) + ".bmp";
        media::EasyScreenCapturer::GetInstance()->CaptureFullScreenAsBmp(filename);
        std::this_thread::sleep_for(std::chrono::milliseconds(50)); //20fps
    }

    return 0;
}