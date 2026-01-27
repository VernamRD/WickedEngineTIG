#include "CPI_Impl.h"
#include <wiProfiler.h>

#include <Core.h>

namespace Giperion
{
    namespace CPI
    {

        void CPI_Impl::Initialize()
        {
            GLThread = std::jthread([this](std::stop_token token) { main(token); });
            Giperion::Core::SetThreadName(GLThread.native_handle(), "GLT");
        }

        void CPI_Impl::Deinitialize()
        {
            GLThread.request_stop();
            GLThread.join();
        }

        bool CPI_Impl::IsInitialized() { return GLThread.native_handle(); }

        void CPI_Impl::main(std::stop_token token)
        {
            while (!token.stop_requested())
            {
                // Simulate game logic thread work:
                std::this_thread::yield();
            }
        }
    }  // namespace CPI
}  // namespace Giperion