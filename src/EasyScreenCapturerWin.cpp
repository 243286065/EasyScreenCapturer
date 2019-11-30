#include "EasyScreenCapturerWin.h"
#include "CaptureStatusCode.h"

#include <windows.h>
// GDI
#pragma comment(lib, "user32.lib")

// D3D9
#include <d3d9.h>
#pragma comment(lib, "D3D9.lib")

// D3D11
#include <d3d11.h>
#include <dxgi1_2.h>
#pragma comment(lib, "D3D11.lib")
#pragma comment(lib, "DXGI.lib")

#include <iostream>

#define RELEASE_RESOURCE(p) \
	if (p)                    \
	{                         \
		p->Release();           \
		p = NULL;               \
	}

namespace media
{
struct DXGIParams
{
	ID3D11Device *m_hDevice;
	ID3D11DeviceContext *m_hContext;
	IDXGIOutputDuplication *m_hDeskDupl;
	DXGI_OUTPUT_DESC m_dxgiOutDesc;
	bool m_bInit = false;
};

EasyScreenCapturerWin::EasyScreenCapturerWin()
{
	m_bSupportDXGI = LoadLibrary("D3D11.dll") && LoadLibrary("DXGI.dll");
	m_bSupportD3D9 = LoadLibrary("D3D9.dll");
}
EasyScreenCapturerWin::~EasyScreenCapturerWin() {}

StatusCode EasyScreenCapturerWin::CaptureScreenAsBmp(const std::string &fileName, uint startX, uint startY, uint width, uint height)
{
	CaptureBmpData bmp;
	StatusCode res = CaptureScreen(bmp, startX, startY, width, height);
	
	if (res != StatusCode::CAPTURE_OK)
	{
		return res;
	}
	else
	{
		//保存到文件
		res = SaveBmpBitsAsFile(fileName, bmp);
		//释放
		free(bmp.m_pixels);
		bmp.free = true;
		return res;
	}
}

StatusCode EasyScreenCapturerWin::CaptureFullScreenAsBmp(const std::string &fileName)
{
	return CaptureScreenAsBmp(fileName, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
}

StatusCode EasyScreenCapturerWin::CaptureScreenWithGDI(CaptureBmpData &bmp, const RectPos &rect)
{
	//获取窗口设备上下文
	HWND pDesktop = GetDesktopWindow();
	HDC winDc = GetDC(pDesktop);
	//创建一个与指定设备兼容的内存设备上下文
	HDC memoryDc = CreateCompatibleDC(winDc);

	//创建位图用于保存屏幕图像
	HBITMAP bitmap = CreateCompatibleBitmap(winDc, rect.width, rect.height);
	//位图信息头
	BITMAPINFOHEADER &bitmapInfoHeader = bmp.m_headerInfo;
	bitmapInfoHeader.biSize = sizeof(BITMAPINFOHEADER);
	bitmapInfoHeader.biPlanes = 1;
	bitmapInfoHeader.biWidth = rect.width;
	bitmapInfoHeader.biHeight = -rect.height;
	bitmapInfoHeader.biBitCount = 32;
	bitmapInfoHeader.biCompression = BI_RGB;
	bitmapInfoHeader.biSizeImage = abs(bitmapInfoHeader.biWidth) * abs(bitmapInfoHeader.biHeight) * sizeof(int);
	bitmapInfoHeader.biXPelsPerMeter = 0;
	bitmapInfoHeader.biYPelsPerMeter = 0;
	bitmapInfoHeader.biClrUsed = 0;
	bitmapInfoHeader.biClrImportant = 0;

	//缓冲区
	if (!bmp.free)
	{
		free(bmp.m_pixels);
	}

	bmp.m_pixels = (uint8_t *)malloc(bitmapInfoHeader.biSizeImage);
	if (!bmp.m_pixels)
	{
		ReleaseDC(pDesktop, winDc);
		DeleteDC(memoryDc);
		DeleteObject(bitmap);
		return StatusCode::CAPTURE_ALLOC_FAILED;
	}
	ZeroMemory(bmp.m_pixels, bitmapInfoHeader.biSizeImage);

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
	int res = GetDIBits(winDc, bitmap, 0, rect.height, bmp.m_pixels, (PBITMAPINFO)&bitmapInfoHeader, DIB_RGB_COLORS); //获取bmp信息

	ReleaseDC(pDesktop, winDc);
	DeleteDC(memoryDc);
	DeleteObject(bitmap);

	if (res == 0)
	{
		free(bmp.m_pixels);
		bmp.free = true;
		return StatusCode::CAPTURE_GET_DIBITS_FAILED;
	}
	else
	{
		return StatusCode::CAPTURE_OK;
	}
}

StatusCode EasyScreenCapturerWin::CaptureScreen(CaptureBmpData& bmp, uint startX, uint startY, uint width, uint height)
{
	const int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	const int screenHeight = GetSystemMetrics(SM_CYSCREEN);

	if (screenWidth != m_screenWidth || screenHeight != m_screenHeight)
	{
		m_bScreenChange = true;
		m_screenWidth = screenWidth;
		m_screenHeight = screenHeight;
	}
	else
	{
		m_bScreenChange = false;
	}

	if (startX < 0 || startY < 0 || startX >= screenWidth || startY >= screenHeight || width <= 0 || height <= 0)
	{
		return StatusCode::CAPTURE_INVALID_PARAMETER;
	}

	if (startX + width > screenWidth)
	{
		width = screenWidth - startX;
	}

	if (startY + height > screenHeight)
	{
		height = screenHeight - startY;
	}

	StatusCode res;
	if (m_bSupportDXGI)
	{
		//支持DXGI
		res = CaptureScreenWithDXGI(bmp, {startX, startY, width, height});
	}
	else if (m_bSupportD3D9)
	{
		//支持D3D9
		res = CaptureScreenWithD3D9(bmp, {startX, startY, width, height});
	}
	else
	{
		res = CaptureScreenWithGDI(bmp, {startX, startY, width, height});
	}

	return res;
}

StatusCode EasyScreenCapturerWin::CaptureScreenWithD3D9(CaptureBmpData &bmp, const RectPos &rect)
{
	LPDIRECT3DDEVICE9 pD3dDevice = NULL;
	LPDIRECT3DSURFACE9 pD3dSurface = NULL;

	//获取句柄
	LPDIRECT3D9 pD3d = Direct3DCreate9(D3D_SDK_VERSION);
	if (pD3d == NULL)
	{
		RELEASE_RESOURCE(pD3dDevice);
		RELEASE_RESOURCE(pD3d);
		return StatusCode::CAPTURE_CREATE_D3D9_FAILED;
	}

	D3DDISPLAYMODE ddm = {0};
	HRESULT hr = pD3d->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &ddm);
	if (hr != D3D_OK)
	{
		RELEASE_RESOURCE(pD3dDevice);
		RELEASE_RESOURCE(pD3d);
		return StatusCode::CAPTURE_D3D_GETDISAPLAYMODE_FAILED;
	}

	D3DPRESENT_PARAMETERS d3dpp;
	ZeroMemory(&d3dpp, sizeof(D3DPRESENT_PARAMETERS));
	d3dpp.Windowed = true;														//true:窗口模式；false:全屏模式
	d3dpp.Flags = D3DPRESENTFLAG_LOCKABLE_BACKBUFFER; //锁定后备缓冲区
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;					//清除后台缓存的内容
	d3dpp.hDeviceWindow = GetDesktopWindow();
	d3dpp.BackBufferFormat = ddm.Format;
	d3dpp.BackBufferHeight = ddm.Height;
	d3dpp.BackBufferWidth = ddm.Width;
	d3dpp.MultiSampleType = D3DMULTISAMPLE_NONE;
	d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;
	d3dpp.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;

	hr = pD3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, d3dpp.hDeviceWindow, D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dpp, &pD3dDevice);
	if (hr != D3D_OK || pD3dDevice == NULL)
	{
		RELEASE_RESOURCE(pD3dDevice);
		RELEASE_RESOURCE(pD3d);
		return StatusCode::CAPTURE_CREATE_D3D9DEVICE_FAILED;
	}

	//创建surface
	hr = pD3dDevice->CreateOffscreenPlainSurface(ddm.Width, ddm.Height, D3DFMT_A8R8G8B8, D3DPOOL_SCRATCH, &pD3dSurface, NULL);
	if (hr != D3D_OK)
	{
		RELEASE_RESOURCE(pD3dDevice);
		RELEASE_RESOURCE(pD3d);
		return StatusCode::CAPTURE_CREATE_D3DSURFACE_FAILED;
	}

	StatusCode res;
	if (pD3dDevice && pD3dSurface)
	{
		hr = pD3dDevice->GetFrontBufferData(0, pD3dSurface);
		if (hr == D3D_OK)
		{
			//可以直接保存成图片
			//hr = D3DXSaveSurfaceToFile("directx9.bmp", D3DXIFF_BMP, pD3dSurface, NULL, NULL);

			//位图信息头
			BITMAPINFOHEADER &bitmapInfoHeader = bmp.m_headerInfo;
			bitmapInfoHeader.biSize = sizeof(BITMAPINFOHEADER);
			bitmapInfoHeader.biPlanes = 1;
			bitmapInfoHeader.biWidth = rect.width;
			bitmapInfoHeader.biHeight = -rect.height; //获取的图像是上下倒置的，因此这里也要做相应处理
			bitmapInfoHeader.biBitCount = 32;
			bitmapInfoHeader.biCompression = BI_RGB;
			bitmapInfoHeader.biSizeImage = abs(bitmapInfoHeader.biWidth) * abs(bitmapInfoHeader.biHeight) * sizeof(int);
			bitmapInfoHeader.biXPelsPerMeter = 0;
			bitmapInfoHeader.biYPelsPerMeter = 0;
			bitmapInfoHeader.biClrUsed = 0;
			bitmapInfoHeader.biClrImportant = 0;

			bmp.m_pixels = (uint8_t *)malloc(bitmapInfoHeader.biSizeImage);
			if (!bmp.m_pixels)
			{
				res = StatusCode::CAPTURE_ALLOC_FAILED;
			}
			else
			{
				ZeroMemory(bmp.m_pixels, bitmapInfoHeader.biSizeImage);
				D3DLOCKED_RECT lockedRect;
				RECT screenRect = {0, 0, 0, 0};
				screenRect.left = rect.x;
				screenRect.right = rect.x + rect.width;
				screenRect.top = rect.y;
				screenRect.bottom = rect.y + rect.height;
				if (FAILED(pD3dSurface->LockRect(&lockedRect, &screenRect, D3DLOCK_NO_DIRTY_UPDATE | D3DLOCK_NOSYSLOCK | D3DLOCK_READONLY)))
				{
					res = StatusCode::CAPTURE_D3D_LOCKRECT_FAILED;
				}
				else
				{
					for (int i = 0; i < rect.height; i++)
					{
						memcpy((BYTE *)bmp.m_pixels + i * rect.width * sizeof(int), (BYTE *)lockedRect.pBits + i * lockedRect.Pitch, rect.width * sizeof(int));
					}
					res = StatusCode::CAPTURE_OK;
				}
			}
		}
		else
		{
			res = StatusCode::CAPTURE_D3D_GETFRONTBUFF_FAILED;
		}
	}

	RELEASE_RESOURCE(pD3dDevice);
	RELEASE_RESOURCE(pD3d);
	RELEASE_RESOURCE(pD3dSurface);
	return res;
}

StatusCode InitDXGI(DXGIParams &params)
{
	// Driver types supported
	D3D_DRIVER_TYPE DriverTypes[] =
			{
					D3D_DRIVER_TYPE_HARDWARE,
					D3D_DRIVER_TYPE_WARP,
					D3D_DRIVER_TYPE_REFERENCE,
			};
	UINT NumDriverTypes = ARRAYSIZE(DriverTypes);

	// Feature levels supported
	D3D_FEATURE_LEVEL FeatureLevels[] =
			{
					D3D_FEATURE_LEVEL_11_0,
					D3D_FEATURE_LEVEL_10_1,
					D3D_FEATURE_LEVEL_10_0,
					D3D_FEATURE_LEVEL_9_1};
	UINT NumFeatureLevels = ARRAYSIZE(FeatureLevels);

	D3D_FEATURE_LEVEL FeatureLevel;

	//创建D3D设备对象
	HRESULT hr;
	for (UINT DriverTypeIndex = 0; DriverTypeIndex < NumDriverTypes; ++DriverTypeIndex)
	{
		hr = D3D11CreateDevice(NULL, DriverTypes[DriverTypeIndex], NULL, 0, FeatureLevels, NumFeatureLevels, D3D11_SDK_VERSION, &params.m_hDevice, &FeatureLevel, &params.m_hContext);
		if (SUCCEEDED(hr))
		{
			break;
		}
	}
	if (FAILED(hr))
	{
		return StatusCode::CAPTURE_D3D_INIT_DXGI_FAILED;
	}

	IDXGIDevice *pDxgiDevice = NULL;
	hr = params.m_hDevice->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void **>(&pDxgiDevice));
	if (FAILED(hr))
	{
		return StatusCode::CAPTURE_D3D_QUERY_DXGIDEVICE_INTERFACE_FAILED;
	}

	//获取DXGI Adapter
	IDXGIAdapter *pDxgiAdapter = NULL;
	hr = pDxgiDevice->GetParent(__uuidof(IDXGIAdapter), reinterpret_cast<void **>(&pDxgiAdapter));
	RELEASE_RESOURCE(pDxgiDevice);
	if (FAILED(hr))
	{
		return StatusCode::CAPTURE_D3D_QUERY_DXGIADAPTER_INTERFACE_FAILED;
	}

	//获取输出
	INT nOutput = 0;
	IDXGIOutput *pDxgiOutput = NULL;
	//如果有多张屏幕，这里就是对屏幕进行枚举
	hr = pDxgiAdapter->EnumOutputs(nOutput, &pDxgiOutput);
	RELEASE_RESOURCE(pDxgiAdapter)
	if (FAILED(hr))
	{
		return StatusCode::CAPTURE_D3D_QUERY_DXGIOUTPUT_INTERFACE_FAILED;
	}
	pDxgiOutput->GetDesc(&params.m_dxgiOutDesc);

	IDXGIOutput1 *pDxgiOutput1 = NULL;
	hr = pDxgiOutput->QueryInterface(__uuidof(IDXGIOutput1), reinterpret_cast<void **>(&pDxgiOutput1));
	RELEASE_RESOURCE(pDxgiOutput);
	if (FAILED(hr))
	{
		return StatusCode::CAPTURE_D3D_QUERY_DXGIOUTPUT1_INTERFACE_FAILED;
	}

	//创建桌面副本
	hr = pDxgiOutput1->DuplicateOutput(params.m_hDevice, &params.m_hDeskDupl);
	RELEASE_RESOURCE(pDxgiOutput1);
	if (FAILED(hr))
	{
		return StatusCode::CAPTURE_D3D_DUPLICATION_OUTPUT_FAILED;
	}

	params.m_bInit = true;
	return StatusCode::CAPTURE_OK;
}

void ReleaseDXGIResource(DXGIParams &params)
{
	if (params.m_hDevice)
		RELEASE_RESOURCE(params.m_hDevice);

	if (params.m_hContext)
		RELEASE_RESOURCE(params.m_hContext);

	if (params.m_hDeskDupl)
		RELEASE_RESOURCE(params.m_hDeskDupl);

	params.m_bInit = false;
}

StatusCode EasyScreenCapturerWin::CaptureScreenWithDXGI(CaptureBmpData &bmp, const RectPos &rect)
{
	static DXGIParams params;
	if (m_bScreenChange)
	{
		if (params.m_bInit)
		{
			ReleaseDXGIResource(params);
		}

		auto res = InitDXGI(params);
		if (res != StatusCode::CAPTURE_OK)
		{
			ReleaseDXGIResource(params);
			return res;
		}
	}

	IDXGIResource *pDesktopResource = NULL;
	DXGI_OUTDUPL_FRAME_INFO frameInfo;
	HRESULT hr = params.m_hDeskDupl->AcquireNextFrame(500, &frameInfo, &pDesktopResource);
    //DXGI初始化后的第一帧很可能是空白状态，这里要去除掉;尝试十次
    int cntTry = 0;
    while (frameInfo.TotalMetadataBufferSize == 0 && cntTry++ < 10) {
        RELEASE_RESOURCE(pDesktopResource);
        params.m_hDeskDupl->ReleaseFrame();
        hr = params.m_hDeskDupl->AcquireNextFrame(500, &frameInfo, &pDesktopResource);
    }
	if (hr == DXGI_ERROR_WAIT_TIMEOUT)
	{
		return StatusCode::CAPTURE_D3D_FRAME_NOCHANGE;
	}
	else if (FAILED(hr))
	{
		//当桌面没有变化时，win10上可能会因为系统优化出现超时
		return StatusCode::CAPTURE_D3D_DXGI_ACQUIRE_NEXT_FRAME_FAILED;
	}

	ID3D11Texture2D *pAcquiredDesktopImage = NULL;
	hr = pDesktopResource->QueryInterface(__uuidof(ID3D11Texture2D), reinterpret_cast<void **>(&pAcquiredDesktopImage));
	RELEASE_RESOURCE(pDesktopResource);
	if (FAILED(hr))
	{
		return StatusCode::CAPTURE_D3D_QUERY_DXGITEXTURE_INTERFACE_FAILED;
	}

	D3D11_TEXTURE2D_DESC frameDescriptor;
	pAcquiredDesktopImage->GetDesc(&frameDescriptor);

	//创建新的buff来填充一帧的图像
	D3D11_TEXTURE2D_DESC desc;
	ID3D11Texture2D *pNewDesktopImage = NULL;
	desc.Width = frameDescriptor.Width;
	desc.Height = frameDescriptor.Height;
	desc.Format = frameDescriptor.Format;
	desc.ArraySize = frameDescriptor.ArraySize;
	desc.BindFlags = 0;
	desc.MiscFlags = 0;
	desc.SampleDesc = frameDescriptor.SampleDesc;
	desc.MipLevels = frameDescriptor.MipLevels;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_FLAG::D3D11_CPU_ACCESS_READ;
	desc.Usage = D3D11_USAGE::D3D11_USAGE_STAGING;
	hr = params.m_hDevice->CreateTexture2D(&desc, NULL, &pNewDesktopImage);
	if (FAILED(hr))
	{
		RELEASE_RESOURCE(pAcquiredDesktopImage);
		params.m_hDeskDupl->ReleaseFrame();
		return StatusCode::CAPTURE_D3D_CREATE_DXGITEXTURE_FAILED;
	}

	//
	// copy next staging buffer to new staging buffer
	//
	params.m_hContext->CopyResource(pNewDesktopImage, pAcquiredDesktopImage);
	RELEASE_RESOURCE(pAcquiredDesktopImage);
	params.m_hDeskDupl->ReleaseFrame();

	//
	// create staging buffer for map bits
	//
	IDXGISurface *pStagingSurf = NULL;
	hr = pNewDesktopImage->QueryInterface(__uuidof(IDXGISurface), (void **)(&pStagingSurf));
	RELEASE_RESOURCE(pNewDesktopImage);
	if (FAILED(hr))
	{
		return StatusCode::CAPTURE_D3D_QUERY_DXGISURFACE_INTERFACE_FAILED;
	}

	//
	// copy bits to user space
	//
	DXGI_MAPPED_RECT mappedRect;
	hr = pStagingSurf->Map(&mappedRect, DXGI_MAP_READ);
	if (SUCCEEDED(hr))
	{
		//位图信息头
		BITMAPINFOHEADER &bitmapInfoHeader = bmp.m_headerInfo;
		bitmapInfoHeader.biSize = sizeof(BITMAPINFOHEADER);
		bitmapInfoHeader.biPlanes = 1;
		bitmapInfoHeader.biWidth = rect.width;
		bitmapInfoHeader.biHeight = -rect.height; //获取的图像是上下倒置的，因此这里也要做相应处理
		bitmapInfoHeader.biBitCount = 32;
		bitmapInfoHeader.biCompression = BI_RGB;
		bitmapInfoHeader.biSizeImage = abs(bitmapInfoHeader.biWidth) * abs(bitmapInfoHeader.biHeight) * sizeof(int);
		bitmapInfoHeader.biXPelsPerMeter = 0;
		bitmapInfoHeader.biYPelsPerMeter = 0;
		bitmapInfoHeader.biClrUsed = 0;
		bitmapInfoHeader.biClrImportant = 0;

		bmp.m_pixels = (uint8_t *)malloc(bitmapInfoHeader.biSizeImage);
		StatusCode res;
		if (!bmp.m_pixels)
		{
			res = StatusCode::CAPTURE_ALLOC_FAILED;
		}
		else
		{
			ZeroMemory(bmp.m_pixels, bitmapInfoHeader.biSizeImage);
			auto windowPos = params.m_dxgiOutDesc.DesktopCoordinates;
			switch (params.m_dxgiOutDesc.Rotation)
			{
			case DXGI_MODE_ROTATION_IDENTITY:
			{
				//直接拷贝
				for (int i = 0; i < rect.height; i++)
				{
					memcpy((BYTE *)bmp.m_pixels + i * rect.width * sizeof(int), (BYTE *)mappedRect.pBits + (i + rect.y) * mappedRect.Pitch + rect.x, rect.width * sizeof(int));
				}
			}
			break;
			case DXGI_MODE_ROTATION_ROTATE90:
			{
				//旋转90度
				for (unsigned int j = 0; j < rect.height; j++)
				{
					for (unsigned int i = 0; i < rect.width; i++)
					{
						//获取的屏幕宽度为正常模式下的高度，高度为正常模式下的宽度
						//映射坐标(x,y)-->(y, winWidth - x - 1)
						//加上窗口偏移量后(x,y)-->(y + rect.y, winWidth - x - rect.x - 1)
						memcpy((BYTE *)bmp.m_pixels + (j * rect.width + i) * sizeof(int), (BYTE *)mappedRect.pBits + ((windowPos.right - i - 1 - rect.x) * windowPos.bottom + j + rect.y) * sizeof(int), sizeof(int));
					}
				}
			}
			break;
			case DXGI_MODE_ROTATION_ROTATE180:
			{
				//旋转180度
				for (unsigned int j = 0; j < rect.height; j++)
				{
					for (unsigned int i = 0; i < rect.width; i++)
					{
						//获取的屏幕宽高为正常模式的宽高
						//映射坐标(x,y)-->(winWidth - x -1, winHeight - y - 1)
						//加上窗口偏移量后(x,y)->(winWidth-x-rect.x -1, winHeight-y-rect.y-1)
						memcpy((BYTE *)bmp.m_pixels + (j * rect.width + i) * sizeof(int), (BYTE *)mappedRect.pBits + ((windowPos.bottom - j - 1 - rect.y) * windowPos.right + windowPos.right - i - rect.x - 1) * sizeof(int), sizeof(int));
					}
				}
			}
			break;
			case DXGI_MODE_ROTATION_ROTATE270:
			{
				//旋转270度
				for (unsigned int j = 0; j < rect.height; j++)
				{
					for (unsigned int i = 0; i < rect.width; i++)
					{
						//获取的屏幕宽度为正常模式下的高度，高度为正常模式下的宽度
						//映射坐标(x,y)-->(winWidth - y - 1, x)
						//加上窗口偏移量后(x,y)->(winWidth - y - 1 - rect.y , x + rect.x)
						memcpy((BYTE *)bmp.m_pixels + (j * rect.width + i) * sizeof(int), (BYTE *)mappedRect.pBits + ((i + rect.x) * windowPos.bottom + windowPos.bottom - j - rect.y - 1) * sizeof(int), sizeof(int));
					}
				}
			}
			break;
			}
			res = StatusCode::CAPTURE_OK;
		}

		pStagingSurf->Unmap();
		RELEASE_RESOURCE(pStagingSurf);
		return res;
	}
	else
	{
		pStagingSurf->Unmap();
		RELEASE_RESOURCE(pStagingSurf);
		return StatusCode::CAPTURE_D3D_DXGI_MAPPED_RECT_FAILED;
	}
}

StatusCode EasyScreenCapturerWin::SaveBmpBitsAsFile(const std::string &fileName, const CaptureBmpData &bmp)
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

	bmpFileHeader.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + bmp.m_headerInfo.biSizeImage;
	bmpFileHeader.bfType = 'MB';
	bmpFileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
	DWORD dwWriten;
	WriteFile(hFile, &bmpFileHeader, sizeof(BITMAPFILEHEADER), &dwWriten, NULL);
	WriteFile(hFile, &bmp.m_headerInfo, sizeof(BITMAPINFOHEADER), &dwWriten, NULL);
	WriteFile(hFile, bmp.m_pixels, bmp.m_headerInfo.biSizeImage, &dwWriten, NULL);
	CloseHandle(hFile);

	return StatusCode::CAPTURE_OK;
}

} // namespace media