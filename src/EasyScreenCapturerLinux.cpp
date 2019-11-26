#include "EasyScreenCapturerLinux.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <cstdint>
#include <cstring>

namespace media {
EasyScreenCapturerLinux::EasyScreenCapturerLinux() {}

EasyScreenCapturerLinux::~EasyScreenCapturerLinux() {}

//截取屏幕，保存为bmp文件
StatusCode EasyScreenCapturerLinux::CaptureScreenAsBmp(
    const std::string &fileName, uint startX, uint startY, uint width,
    uint height) {
  CaptureBmpData bmp;
  auto res = CaptureScreenWithX11(bmp, {startX, startY, width, height});
  if (res != StatusCode::CAPTURE_OK) {
    return res;
  }

  //保存文件
  res = SaveBmpBitsAsFile(fileName, bmp);
  if (!bmp.free) {
    free(bmp.m_pixels);
    bmp.m_pixels = NULL;
    bmp.free = true;
  }

  return res;
}

StatusCode EasyScreenCapturerLinux::CaptureFullScreenAsBmp(
    const std::string &fileName) {
  return CaptureScreenAsBmp(fileName, 0, 0, 0, 0);
}

// 使用x11捕获屏幕
StatusCode EasyScreenCapturerLinux::CaptureScreenWithX11(CaptureBmpData &bmp,
                                                         const RectPos &rect) {
  Display *display = XOpenDisplay(nullptr);
  if (!display) {
    return StatusCode::CAPTURE_X11_OPEN_DISPLAY_FAILED;
  }

  Window root = DefaultRootWindow(display);

  XWindowAttributes attributes = {0};
  XGetWindowAttributes(display, root, &attributes);

  uint x = rect.x;
  uint y = rect.y;
  uint width = rect.width;
  uint height = rect.height;
  if (x == 0 && y == 0 && width == 0 && height == 0) {
    width = attributes.width;
    height = attributes.height;
  }

  if (x > attributes.width || y > attributes.height) {
    XCloseDisplay(display);
    return StatusCode::CAPTURE_INVALID_PARAMETER;
  }

  if (x + width > attributes.width) {
    width = attributes.width - x;
    height = attributes.height - y;
  }

  XImage *img =
      XGetImage(display, root, x, y, width, height, AllPlanes, ZPixmap);
  if (!img) {
    XCloseDisplay(display);
    return StatusCode::CAPTURE_X11_GET_IMAGE_FAILED;
  }

  // int BitsPerPixel = img->bits_per_pixel;
  bmp.m_headerInfo.biSize = sizeof(BITMAPINFOHEADER);
  bmp.m_headerInfo.biPlanes = 1;
  bmp.m_headerInfo.biWidth = width;
  bmp.m_headerInfo.biHeight =
      -abs(height);  //获取的图像是上下倒置的，因此这里也要做相应处理
  bmp.m_headerInfo.biBitCount = 32;
  bmp.m_headerInfo.biCompression = 0L;  // BI_RGB
  bmp.m_headerInfo.biSizeImage = abs(bmp.m_headerInfo.biWidth) *
                                 abs(bmp.m_headerInfo.biHeight) * sizeof(int);
  bmp.m_headerInfo.biXPelsPerMeter = 0;
  bmp.m_headerInfo.biYPelsPerMeter = 0;
  bmp.m_headerInfo.biClrUsed = 0;
  bmp.m_headerInfo.biClrImportant = 0;

  if (!bmp.free) {
    free(bmp.m_pixels);
  }
  bmp.free = false;
  bmp.m_pixels = (uint8_t *)malloc(bmp.m_headerInfo.biSizeImage * sizeof(int));
  memcpy(bmp.m_pixels, img->data, bmp.m_headerInfo.biSizeImage);
  XDestroyImage(img);
  XCloseDisplay(display);
  return StatusCode::CAPTURE_OK;
}

StatusCode EasyScreenCapturerLinux::SaveBmpBitsAsFile(
    const std::string &fileName, const CaptureBmpData &bmp) {
  FILE *outfile = fopen(fileName.c_str(), "wb");

  BITMAPFILEHEADER bmpFileHeader;
  bmpFileHeader.bfReserved1 = 0;
  bmpFileHeader.bfReserved2 = 0;

  bmpFileHeader.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) +
                         bmp.m_headerInfo.biSizeImage;
  bmpFileHeader.bfType = 'MB';
  bmpFileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
  uint32_t dwWriten;
  fwrite(&bmpFileHeader, sizeof(char), sizeof(BITMAPFILEHEADER), outfile);
  fwrite(&bmp.m_headerInfo, sizeof(char), sizeof(BITMAPINFOHEADER), outfile);
  fwrite(bmp.m_pixels, sizeof(char), bmp.m_headerInfo.biSizeImage, outfile);
  fclose(outfile);
  outfile = NULL;
  return StatusCode::CAPTURE_OK;
}

}  // namespace media