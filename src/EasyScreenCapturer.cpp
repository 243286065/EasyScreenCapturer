#include "EasyScreenCapturer.h"

#ifdef OS_WIN
#include "EasyScreenCapturerWin.h"
#elif defined(OS_UNIX)
#include "EasyScreenCapturerLinux.h"
#else
#include "EasyScreenCapturerFaker.h"
#endif

namespace media
{
std::shared_ptr<EasyScreenCapturer> EasyScreenCapturer::m_pInstance = nullptr;

std::shared_ptr<EasyScreenCapturer> EasyScreenCapturer::GetInstance()
{
  if (!m_pInstance)
  {
#ifdef OS_WIN
    m_pInstance.reset(new EasyScreenCapturerWin());
#elif defined(OS_UNIX)
    m_pInstance.reset(new EasyScreenCapturerLinux());
#else
    m_pInstance.reset(new EasyScreenCapturerFaker());
#endif
  }

  return m_pInstance;
}

void EasyScreenCapturer::FreeCaptureBmpData(CaptureBmpData& bmp)
{
  if(!bmp.m_free)
  {
    free(bmp.m_pixels);
    bmp.m_pixels = NULL;
    bmp.m_free = true;
  }
}
} // namespace media