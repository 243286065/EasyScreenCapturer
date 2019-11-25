#include "EasyScreenCapturer"

namespace media
{

class EasyScreenCaptureFaker : public EasyScreenCapture
{
public:
	EasyScreenCaptureFaker() {}
	~EasyScreenCaptureFaker() {}

	//截取屏幕，保存为bmp文件
	StatusCode CaptureScreenAsBmp(const std::string &fileName, int startX, int startY, int width, int height) override {}

  StatusCode CaptureFullScreenAsBmp(const std::string &fileName) override{}
};

} // namespace media
