#include "Task.h"

namespace wi::compute
{
    TaskHandle::TaskHandle::TaskHandle(const TaskPtr& task) { m_handle = HandleType(task); }

    const std::string& TaskHandle::GetName() const { return m_handle->GetName(); }

    const wi::vector<TaskHandle>& TaskHandle::GetPrerequisites() const { return GetTaskPtr()->GetPrerequisites(); }

    TaskPtr TaskHandle::GetTaskPtr() const { return m_handle; }

    Task::Task(std::string&& name)
        : m_name(std::move(name))
    {
    }

    Task::Task(std::string&& name, const wi::vector<TaskHandle>& prerequisites)
        : m_name(std::move(name))
        , m_prerequisites(prerequisites)
    {
    }

    Task::Task(std::string&& name, std::initializer_list<TaskHandle> prerequisites)
        : m_name(std::move(name))
        , m_prerequisites(prerequisites)
    {
    }

    TaskHandle Task::InitHandle(const TaskPtr& thisPtr) const
    {
        assert(thisPtr.get() == this);
        TaskHandle handle(thisPtr);
        return handle;
    }

    const wi::vector<TaskHandle>& Task::GetPrerequisites() const { return m_prerequisites; }

}  // namespace wi::Compute
