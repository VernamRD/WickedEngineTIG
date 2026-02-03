#pragma once

#include "Task.h"

namespace wi::compute
{
    class CPUTask : public Task
    {
    public:
        using Task::Task;

        virtual void Initialize_CPU() {}
        virtual void Execute_CPU() {}

    private:
    };

}  // namespace wi::compute
