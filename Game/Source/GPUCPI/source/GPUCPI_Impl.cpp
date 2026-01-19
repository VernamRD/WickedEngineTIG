#include "GPUCPI_Impl.h"
#include <wiProfiler.h>

#include <Core.h>

namespace Giperion
{
    namespace GPUCPI
    {

        void GPUCPI_Impl::Initialize()
        {
            GLThread = std::jthread([this](std::stop_token token) { main(token); });
            Giperion::Core::SetThreadName(GLThread.native_handle(), "GLT");
        }

        void GPUCPI_Impl::Deinitialize()
        {
            GLThread.request_stop();
            GLThread.join();
        }

        bool GPUCPI_Impl::IsInitialized() { return GLThread.native_handle(); }

        void GPUCPI_Impl::main(std::stop_token token)
        {
            while (!token.stop_requested())
            {
                // Simulate game logic thread work:
                std::this_thread::yield();
            }
        }
    }  // namespace GPUCPI
}  // namespace Giperion