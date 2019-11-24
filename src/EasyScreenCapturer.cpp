#include "EasyScreenCapturer.h"

#ifdef OS_WIN
#include "EasyScreenCapturerWin.h"
#elif define(OS_UNIX)
#else
#include "EasyScreenCaptureFaker.h"
#endif

namespace media
{
std::shared_ptr<EasyScreenCapture> EasyScreenCapture::m_pInstance = nullptr;

std::shared_ptr<EasyScreenCapture> EasyScreenCapture::GetInstance()
{
	if (!m_pInstance)
	{
#ifdef OS_WIN
		m_pInstance.reset(new EasyScreenCaptureWin());
#elif define OS_UNIX
		//m_pInstance.reset(new EasyScreenCaptureFaker());
#else
	m_pInstance.reset(new EasyScreenCapture());
#endif
	}

	return m_pInstance;
}
} // namespace media