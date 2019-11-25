#ifndef _SRC_EASY_SCREEN_CAPTURER_WIN_H_
#define _SRC_EASY_SCREEN_CAPTURER_WIN_H_

#include "EasyScreenCapturer.h"

#include <windows.h>
#include <memory>
#include <string>

namespace media
{

struct RectPos
{
	int x;
	int y;
	int width;
	int height;
};

struct CaptureBmpData
{
	LPVOID data;
	int len;
	bool free = true;
	BITMAPINFOHEADER bitmapHeader;
};

class EasyScreenCaptureWin : public EasyScreenCapture
{
public:
	EasyScreenCaptureWin();
	~EasyScreenCaptureWin();

	//截取屏幕，保存为bmp文件
	StatusCode CaptureScreenAsBmp(const std::string &fileName, int startX, int startY, int width, int height) override;

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
};

} // namespace media

#endif // _SRC_EASY_SCREEN_CAPTURER_WIN_H_