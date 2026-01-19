#include "GameLogicThread.h"
#include "wiProfiler.h"

#include <Core.h>

void GameLogicThread::Initialize()
{
    GLThread = std::jthread([this](std::stop_token token) { main(token); });
    Giperion::Core::SetThreadName(GLThread.native_handle(), "GLT");
}

void GameLogicThread::Deinitialize()
{
    GLThread.request_stop();
    GLThread.join();
}

bool GameLogicThread::IsInitialized() { return GLThread.native_handle(); }

void GameLogicThread::main(std::stop_token token)
{
    while (!token.stop_requested())
    {
        // Begin final compositing:
        // wi::profiler::BeginGLTFrame();

        // Simulate game logic thread work:
        std::this_thread::yield();

        // wi::profiler::EndGLTFrame();
    }
}
