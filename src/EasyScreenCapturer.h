#ifndef _SRC_EASY_SCREEN_CAPTURER_H_
#define _SRC_EASY_SCREEN_CAPTURER_H_

#include "CaptureStatusCode.h"

#include <memory>

namespace media
{

class __declspec(dllexport) EasyScreenCapture {
public:
	virtual ~EasyScreenCapture() {}

	//截取屏幕，保存为bmp文件
	virtual StatusCode CaptureScreenAsBmp(const std::string &fileName, const int startX, const int startY, const int width, const int height) {
        return StatusCode::CAPTURE_NO_IMPLEMENT;
    }

    virtual StatusCode CaptureFullScreenAsBmp(const std::string &fileName) {
        return StatusCode::CAPTURE_NO_IMPLEMENT;
    }

	static std::shared_ptr<EasyScreenCapture> GetInstance();

protected:
	EasyScreenCapture() {}

private:
	static std::shared_ptr<EasyScreenCapture> m_pInstance;
};

}

#endif