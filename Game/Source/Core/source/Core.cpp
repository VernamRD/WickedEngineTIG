#include "Core.h"

#include <string>

#if defined(_WIN32) || defined(_WIN64)

#ifndef _AMD64_
#define _AMD64_
#endif

#include <processthreadsapi.h>
#include <winerror.h>
#elif defined(__linux__)
#include <pthread.h>
#endif

namespace Giperion
{
    namespace Core
    {
#if defined(_WIN32) || defined(_WIN64)
        bool SetThreadName(std::thread::native_handle_type handle, const char* name)
        {
            std::wstring w_name(name, name + strlen(name));
            return SUCCEEDED(SetThreadDescription(handle, w_name.c_str()));
        }
#elif defined(__linux__)
        bool SetThreadName(std::thread::native_handle_type handle, const char* name)
        {
            return pthread_setname_np(handle, name) == 0;
        }
#endif

    }  // namespace Core
}  // namespace Giperion