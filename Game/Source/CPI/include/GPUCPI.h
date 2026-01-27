#pragma once

#include "CPITask.h"
#include <WickedEngine.h>

namespace Giperion
{
    namespace CPI
    {
        // Graphics Processing Units Compute Pipeline Interface
        class CPI
        {
        public:
            virtual ~CPI() = default;

            virtual void Initialize() = 0;
            virtual void Deinitialize() = 0;
            virtual bool IsInitialized() = 0;
        };

        void Init();
        bool IsInitialized();
        void Deinitialize();
        CPI& Get();

        class CPUTask : public CPITask
        {
            
        };

        class GPUTask : public CPITask
        {
            
        };
    }  // namespace CPI
}  // namespace Giperion
