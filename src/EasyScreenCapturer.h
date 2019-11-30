#ifndef _SRC_EASY_SCREEN_CAPTURER_H_
#define _SRC_EASY_SCREEN_CAPTURER_H_

#include "CaptureStatusCode.h"

#include <memory>

#ifdef _WIN32
#include <windows.h>
#endif


#ifdef _WIN32
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#endif

namespace media
{

#ifdef _WIN32
typedef unsigned int uint;
#else
typedef uint16_t WORD;
typedef uint32_t DWORD;
typedef int32_t LONG;

#pragma pack(push, 1)
struct BITMAPFILEHEADER
{										//位图文件头,14字节
	WORD bfType;			//  指定文件类型，必须是0x424D，即字符串“BM”，也就是说所有.bmp文件的头两个字节都是“BM”。
	DWORD bfSize;			//   位图文件的大小，包括这14个字节，以字节为单位
	WORD bfReserved1; //   位图文件保留字，必须为0
	WORD bfReserved2; //   位图文件保留字，必须为0
	DWORD bfOffBits;	//   位图数据的起始位置，以相对于位图，
										//   文件头的偏移量表示，以字节为单位
};
#pragma pack(pop)
struct
		BITMAPINFOHEADER
{									 //这个结构的长度是固定的，为40个字节,可以自己算一下，DWORD、LONG4个字节，WORD两个字节
	DWORD biSize;		 //指定这个结构的长度，为40
	LONG biWidth;		 //指定图象的宽度，单位是象素。
	LONG biHeight;	 //指定图象的高度，单位是象素。
	WORD biPlanes;	 //必须是1，不用考虑。
	WORD biBitCount; /*指定表示颜色时要用到的位数，常用的值为1(黑白二色图),
                      4(16色图), 8(256色),
                      24(真彩色图)(新的.bmp格式支持32位色，这里就不做讨论了)。*/
	DWORD
	biCompression; /*指定位图是否压缩，有效的值为BI_RGB，BI_RLE8，BI_RLE4，
                                             BI_BITFIELDS(都是一些Windows定义好的常量)。要说明的是，
                                             Windows位图可以采用RLE4，和RLE8的压缩格式，但用的不多。
                                             我们今后所讨论的只有第一种不压缩的情况，即biCompression为BI_RGB的情况。*/
	DWORD
	biSizeImage;					/*指定实际的位图数据占用的字节数，其实也可以从以下的公式中计算出来：
biSizeImage=biWidth’ × biHeight
要注意的是：上述公式中的biWidth’必须是4的整倍数(所以不是biWidth，而是biWidth’，
表示大于或等于biWidth的，最接近4的整倍数。举个例子，如果biWidth=240，则biWidth’=240；
如果biWidth=241，biWidth’=244)。如果biCompression为BI_RGB，则该项可能为零*/
	LONG biXPelsPerMeter; //指定目标设备的水平分辨率，单位是每米的象素个数
	LONG biYPelsPerMeter; //指定目标设备的垂直分辨率，单位同上。
	DWORD
	biClrUsed; //指定本图象实际用到的颜色数，如果该值为零，则用到的颜色数为2的biBitCount指数次幂
	DWORD
	biClrImportant; //指定本图象中重要的颜色数，如果该值为零，则认为所有的颜色都是重要的。
};
#endif

struct CaptureBmpData
{
	uint8_t *m_pixels;
	bool free = true;
	BITMAPINFOHEADER m_headerInfo;
};

struct RectPos
{
	uint x;
	uint y;
	uint width;
	uint height;
};

class EXPORT EasyScreenCapturer
{
public:
	virtual ~EasyScreenCapturer() {}

	//截取屏幕，保存为bmp文件
	virtual StatusCode CaptureScreenAsBmp(const std::string &fileName, uint startX, uint startY, uint width, uint height) = 0;

	//全屏截图，保存为bmp文件
	virtual StatusCode CaptureFullScreenAsBmp(const std::string &fileName) = 0;

	//截图,保存在内存中
	virtual StatusCode CaptureScreen(CaptureBmpData& bmp, uint startX, uint startY, uint width, uint height) = 0;

	static std::shared_ptr<EasyScreenCapturer> GetInstance();

protected:
	EasyScreenCapturer() {}

private:
	static std::shared_ptr<EasyScreenCapturer> m_pInstance;
};

} // namespace media

#endif