#include "CPITask.h"

namespace Giperion
{
    namespace GPUCPI
    {
        CPITaskHandle::CPITaskHandle::CPITaskHandle(const CPITaskPtr& task)
        {
            m_handle = HandleType(task);
        }

        CPITask::CPITask(const wi::vector<CPITaskHandle>& prerequisites)
            : m_prerequisites(prerequisites)
        {
        }

        const wi::vector<CPITaskHandle>& CPITask::GetPrerequisites() const
        {
            return m_prerequisites;
        }

    }  // namespace GPUCPI
}  // namespace Giperion
