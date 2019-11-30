#ifndef _SRC_EASY_SCREEN_CAPTURER_LINUX_H_
#define _SRC_EASY_SCREEN_CAPTURER_LINUX_H_

#include "EasyScreenCapturer.h"

#include <memory>
#include <string>
#include <vector>

namespace media
{

class EasyScreenCapturerLinux : public EasyScreenCapturer
{
public:
  EasyScreenCapturerLinux();
  ~EasyScreenCapturerLinux();

  //截取屏幕，保存为bmp文件
  StatusCode CaptureScreenAsBmp(const std::string &fileName, uint startX,
                                uint startY, uint width, uint height) override;

  StatusCode CaptureFullScreenAsBmp(const std::string &fileName) override;

  StatusCode CaptureScreen(CaptureBmpData& bmp, uint startX, uint startY, uint width, uint height) override;

private:
  // 使用x11捕获屏幕
  StatusCode CaptureScreenWithX11(CaptureBmpData &bmp, const RectPos &rect);

  //保存成为文件
  StatusCode SaveBmpBitsAsFile(const std::string &fileName,
                               const CaptureBmpData &bmp);
};

} // namespace media

#endif // _SRC_EASY_SCREEN_CAPTURER_LINUX_H_