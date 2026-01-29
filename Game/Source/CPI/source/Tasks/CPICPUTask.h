#pragma once

#include "CPITask.h"

namespace Giperion
{
    namespace CPI
    {
        class CPUTask : public CPITask
        {
        public:
            using CPITask::CPITask;

            virtual void Initialize_CPU() {}
            virtual void Execute_CPU() {}

        private:
        };

    }  // namespace CPI

}  // namespace Giperion