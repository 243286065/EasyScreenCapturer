#ifndef _SRC_EASY_SCREEN_CAPTURER_H_
#define _SRC_EASY_SCREEN_CAPTURER_H_

#include "CaptureStatusCode.h"

#include <memory>

#ifdef _win32
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#endif

namespace media
{

struct RectPos
{
	uint x;
	uint y;
	uint width;
	uint height;
};

class EXPORT EasyScreenCapturer {
public:
	virtual ~EasyScreenCapturer() {}

	//截取屏幕，保存为bmp文件
	virtual StatusCode CaptureScreenAsBmp(const std::string &fileName, uint startX, uint startY, uint width, uint height) = 0;

    virtual StatusCode CaptureFullScreenAsBmp(const std::string &fileName) = 0;

	static std::shared_ptr<EasyScreenCapturer> GetInstance();

protected:
	EasyScreenCapturer() {}

private:
	static std::shared_ptr<EasyScreenCapturer> m_pInstance;
};

}

#endif