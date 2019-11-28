#ifndef _SRC_EASY_SCREEN_CAPTURER_WIN_H_
#define _SRC_EASY_SCREEN_CAPTURER_WIN_H_

#include "EasyScreenCapturer.h"

#include <memory>
#include <string>
#include <windows.h>

namespace media
{
class EasyScreenCapturerWin : public EasyScreenCapturer
{
public:
	EasyScreenCapturerWin();
	~EasyScreenCapturerWin();

	//截取屏幕，保存为bmp文件
	StatusCode CaptureScreenAsBmp(const std::string &fileName, uint startX, uint startY, uint width, uint height) override;

	StatusCode CaptureFullScreenAsBmp(const std::string &fileName) override;

private:
	// 使用GDI捕获屏幕
	StatusCode CaptureScreenWithGDI(CaptureBmpData &bmp, const RectPos &rect);

	// 使用D3D9捕获屏幕
	StatusCode CaptureScreenWithD3D9(CaptureBmpData &bmp, const RectPos &rect);

	// 使用DXGI捕获屏幕
	StatusCode CaptureScreenWithDXGI(CaptureBmpData &bmp, const RectPos &rect);

	//保存成为文件
	StatusCode SaveBmpBitsAsFile(const std::string &fileName, const CaptureBmpData &bmp);

	//是否支持DXGI
	bool m_bSupportDXGI = false;

	//是否支持D3D9
	bool m_bSupportD3D9 = false;

	//屏幕分辨率
	int m_screenWidth = 0;
	int m_screenHeight = 0;
	bool m_bScreenChange = false;
};

} // namespace media

#endif // _SRC_EASY_SCREEN_CAPTURER_WIN_H_