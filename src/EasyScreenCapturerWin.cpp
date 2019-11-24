#include "EasyScreenCapturerWin.h"
#include "CaptureStatusCode.h"

//#include <windef.h>
#include <windows.h>
// GDI
//#include <Winuser.h>
#pragma comment(lib, "user32.lib")

namespace media
{
EasyScreenCaptureWin::EasyScreenCaptureWin() {}
EasyScreenCaptureWin::~EasyScreenCaptureWin() {}

StatusCode EasyScreenCaptureWin::CaptureScreenAsBmp(const std::string &fileName, const int startX, const int startY, const int width, const int height)
{
    const int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    const int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    if (startX < 0 || startY < 0 || startX >= screenWidth || startY >= screenHeight || width <= 0 || height <= 0 || width > screenWidth || height > screenHeight)
    {
        return StatusCode::CAPTURE_INVALID_PARAMETER;
    }

    CaptureBmpData bmp;
    auto res = CaptureScreenWithGDI(bmp, {startX, startY, width, height});
    if (res != StatusCode::CAPTURE_OK)
    {
        return res;
    }
    else
    {
        //保存到文件
        res = SaveBmpBitsAsFile(fileName, bmp);
        //释放
        free(bmp.data);
        bmp.free = true;
        return SaveBmpBitsAsFile(fileName, bmp);
    }
}

StatusCode EasyScreenCaptureWin::CaptureFullScreenAsBmp(const std::string &fileName) {
    return CaptureScreenAsBmp(fileName, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
}

StatusCode EasyScreenCaptureWin::CaptureScreenWithGDI(CaptureBmpData &bmp, const RectPos &rect)
{
    //获取窗口设备上下文
    HWND pDesktop = GetDesktopWindow();
    HDC winDc = GetDC(pDesktop);
    //创建一个与指定设备兼容的内存设备上下文
    HDC memoryDc = CreateCompatibleDC(winDc);

    //创建位图用于保存屏幕图像
    HBITMAP bitmap = CreateCompatibleBitmap(winDc, rect.width, rect.height);
    //位图信息头
    BITMAPINFOHEADER &bitmapInfoHeader = bmp.bitmapHeader;
    bitmapInfoHeader.biSize = sizeof(BITMAPINFOHEADER);
    bitmapInfoHeader.biPlanes = 1;
    bitmapInfoHeader.biWidth = rect.width;
    bitmapInfoHeader.biHeight = rect.height;
    bitmapInfoHeader.biBitCount = 32;
    bitmapInfoHeader.biCompression = BI_RGB;
    bitmapInfoHeader.biSizeImage = abs(bitmapInfoHeader.biWidth)*abs(bitmapInfoHeader.biHeight) * sizeof(int);;
    bitmapInfoHeader.biXPelsPerMeter = 0;
    bitmapInfoHeader.biYPelsPerMeter = 0;
    bitmapInfoHeader.biClrUsed = 0;
    bitmapInfoHeader.biClrImportant = 0;

    //缓冲区
    if (!bmp.free)
    {
        free(bmp.data);
    }

    bmp.len = bitmapInfoHeader.biSizeImage;
    bmp.data = (LPVOID)malloc(bmp.len);
    if (!bmp.data)
    {
        ReleaseDC(pDesktop, winDc);
        DeleteDC(memoryDc);
        DeleteObject(bitmap);
        return StatusCode::CAPTURE_ALLOC_FAILED;
    }
    memset(bmp.data, 0, bmp.len);

    //替换目标设备中的位图对象
    HBITMAP oldBitMap = (HBITMAP)SelectObject(memoryDc, bitmap);
    //对指定的源设备环境区域中的像素进行位块（bit_block）转换，以传送到目标设备环境
    if (!BitBlt(memoryDc, 0, 0, rect.width, rect.height, winDc, rect.x, rect.y, SRCCOPY | CAPTUREBLT))
    {
        ReleaseDC(pDesktop, winDc);
        DeleteDC(memoryDc);
        DeleteObject(bitmap);
        return StatusCode::CAPTURE_ERROR_BITBIT;
    }
    //获取指定兼容位图的位，然后将其作一个DIB—设备无关位图（Device-Independent Bitmap）使用的指定格式复制到一个缓冲区中
    int res = GetDIBits(winDc, bitmap, 0, rect.height, bmp.data, (PBITMAPINFO)&bitmapInfoHeader, DIB_RGB_COLORS); //获取bmp信息

    ReleaseDC(pDesktop, winDc);
    DeleteDC(memoryDc);
    DeleteObject(bitmap);

    if (res == 0)
    {
        free(bmp.data);
        bmp.free = true;
        return StatusCode::CAPTURE_GET_DIBITS_FAILED;
    }
    else
    {
        return StatusCode::CAPTURE_OK;
    }
}

StatusCode EasyScreenCaptureWin::SaveBmpBitsAsFile(const std::string &fileName, const CaptureBmpData &bmp)
{
    // 创建一个文件来保存文件截图
    HANDLE hFile = CreateFileA(
        fileName.c_str(),
        GENERIC_WRITE,
        0,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL);

    BITMAPFILEHEADER bmpFileHeader;
    bmpFileHeader.bfReserved1 = 0;
    bmpFileHeader.bfReserved2 = 0;

    bmpFileHeader.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + bmp.bitmapHeader.biSizeImage;
    bmpFileHeader.bfType = 'MB';
    bmpFileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    DWORD dwWriten;
    WriteFile(hFile, &bmpFileHeader, sizeof(BITMAPFILEHEADER), &dwWriten, NULL);
    WriteFile(hFile, &bmp.bitmapHeader, sizeof(BITMAPINFOHEADER), &dwWriten, NULL);
    WriteFile(hFile, bmp.data, bmp.bitmapHeader.biSizeImage, &dwWriten, NULL);
    //CloseHandle(hFile);

    return StatusCode::CAPTURE_OK;
}

} // namespace media