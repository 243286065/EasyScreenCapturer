#ifndef _SRC_EASY_SCREEN_CAPTURER_H_
#define _SRC_EASY_SCREEN_CAPTURER_H_

#include "CaptureStatusCode.h"

#include <memory>

#ifdef OS_WIN
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#endif

namespace media
{

#ifdef OS_WIN
 typedef unsigned int uint;
#endif

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

    //全屏截图，保存为bmp文件
    virtual StatusCode CaptureFullScreenAsBmp(const std::string &fileName) = 0;

	static std::shared_ptr<EasyScreenCapturer> GetInstance();

protected:
	EasyScreenCapturer() {}

private:
	static std::shared_ptr<EasyScreenCapturer> m_pInstance;
};

}

#endif