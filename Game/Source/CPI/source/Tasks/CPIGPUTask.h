#pragma once

#include "CPITask.h"
#include <wiGraphicsDevice.h>

namespace Giperion
{
    namespace CPI
    {
        class GPUTask : public CPITask
        {
        public:
            using CPITask::CPITask;
            
            virtual void Initialize_GPU(wi::graphics::GraphicsDevice* device, wi::graphics::CommandList cmd) {}
            virtual void Execute_GPU(wi::graphics::GraphicsDevice* device, wi::graphics::CommandList cmd) {}

        private:
        };

    }  // namespace CPI

}  // namespace Giperion