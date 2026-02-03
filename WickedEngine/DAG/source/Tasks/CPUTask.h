#pragma once

#include "Task.h"

namespace Giperion
{
    namespace Compute
    {
        class CPUTask : public Task
        {
        public:
            using Task::Task;

            virtual void Initialize_CPU() {}
            virtual void Execute_CPU() {}

        private:
        };

    }  // namespace DAG

}  // namespace Giperion