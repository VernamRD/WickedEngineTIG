#include "Tasks/CPITask.h"

namespace Giperion
{
    namespace CPI
    {
        CPITaskHandle::CPITaskHandle::CPITaskHandle(const CPITaskPtr& task)
        {
            m_handle = HandleType(task);
        }

        const std::string& CPITaskHandle::GetName() const { return m_handle->GetName(); }

        const wi::vector<CPITaskHandle>& CPITaskHandle::GetPrerequisites() const
        {
            return GetTaskPtr()->GetPrerequisites();
        }

        CPITaskPtr CPITaskHandle::GetTaskPtr() const { return m_handle; }

        CPITask::CPITask(std::string&& name)
            : m_name(std::move(name))
        {
        }

        CPITask::CPITask(std::string&& name, const wi::vector<CPITaskHandle>& prerequisites)
            : m_name(std::move(name))
            , m_prerequisites(prerequisites)
        {
        }

        CPITask::CPITask(std::string&& name, std::initializer_list<CPITaskHandle> prerequisites)
            : m_name(std::move(name))
            , m_prerequisites(prerequisites)
        {
        }

        CPITaskHandle CPITask::InitHandle(CPITaskPtr thisPtr) const
        {
            assert(thisPtr.get() == this);
            CPITaskHandle handle(thisPtr);
            return handle;
        }

        const wi::vector<CPITaskHandle>& CPITask::GetPrerequisites() const { return m_prerequisites; }

    }  // namespace CPI
}  // namespace Giperion
