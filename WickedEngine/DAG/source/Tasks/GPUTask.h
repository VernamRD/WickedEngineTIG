#pragma once

#include "Task.h"
#include <wiGraphicsDevice.h>

namespace wi::compute
{
    class GPUTask : public Task
    {
    public:
        using Task::Task;

        virtual void Initialize_GPU(wi::graphics::GraphicsDevice* device, wi::graphics::CommandList cmd) {}
        virtual void Execute_GPU(wi::graphics::GraphicsDevice* device, wi::graphics::CommandList cmd) {}

    private:
    };

}  // namespace wi::compute
