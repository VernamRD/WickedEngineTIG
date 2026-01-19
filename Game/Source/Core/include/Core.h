#pragma once

#include <thread>

namespace Giperion
{
    namespace Core
    {
        bool SetThreadName(std::thread::native_handle_type handle, const char* name);
    }  // namespace Core

}  // namespace Giperion