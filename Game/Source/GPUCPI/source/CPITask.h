#pragma once

#include <wiAllocator.h>

namespace Giperion
{
    namespace GPUCPI
    {
        class CPITask;

        using CPITaskPtr = std::shared_ptr<CPITask>;

        class CPITaskHandle
        {
            using HandleType = CPITaskPtr;

        public:
            CPITaskHandle() = delete;
            CPITaskHandle(const CPITaskPtr& task);

            HandleType GetHandle() const { return m_handle; }

        private:
            HandleType m_handle;
        };

        class CPITask
        {
        public:
            CPITask(const wi::vector<CPITaskHandle>& prerequisites);

            const wi::vector<CPITaskHandle>& GetPrerequisites() const;

        private:
            wi::vector<CPITaskHandle> m_prerequisites;
        };
    }  // namespace GPUCPI

}  // namespace Giperion