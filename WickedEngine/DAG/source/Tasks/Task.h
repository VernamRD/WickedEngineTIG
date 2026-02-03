#pragma once

#include <wiAllocator.h>
#include <type_traits>
#include <string>

namespace wi::compute
{
    class Task;
    class TaskHandle;

    using PrerequisitesType = wi::vector<TaskHandle>;
    using TaskPtr = std::shared_ptr<Task>;

    class TaskHandle
    {
    public:
        using HandleType = TaskPtr;

        TaskHandle() = delete;
        TaskHandle(const TaskPtr& task);

        [[nodiscard]] const std::string& GetName() const;
        [[nodiscard]] const PrerequisitesType& GetPrerequisites() const;

        [[nodiscard]] bool IsValid() const { return m_handle != nullptr; }

        bool operator==(const TaskHandle&) const = default;
        [[nodiscard]] inline size_t hash_value() const { return std::hash<Task*>{}(m_handle.get()); }

    private:
        [[nodiscard]] TaskPtr GetTaskPtr() const;

        HandleType m_handle;
    };

    enum class ETaskType : uint8_t
    {
        Undefined,
        CPU,
        GPU
    };

    class Task
    {
        friend class TaskFactory;

    public:
        Task() = delete;
        virtual ~Task() = default;

        [[nodiscard]] const PrerequisitesType& GetPrerequisites() const;
        [[nodiscard]] const std::string& GetName() const { return m_name; }
        [[nodiscard]] ETaskType GetTaskType() const { return m_taskType; }

    protected:
        Task(std::string&& inName);
        Task(std::string&& inName, const wi::vector<TaskHandle>& prerequisites);
        Task(std::string&& inName, std::initializer_list<TaskHandle> prerequisites);

    private:
        [[nodiscard]] TaskHandle InitHandle(const TaskPtr& thisPtr) const;

        std::string m_name;
        PrerequisitesType m_prerequisites;
        ETaskType m_taskType = ETaskType::Undefined;
    };

}  // namespace wi::compute

namespace std
{
    template <>
    struct hash<wi::compute::TaskHandle>
    {
        size_t operator()(const wi::compute::TaskHandle& handle) const noexcept { return handle.hash_value(); }
    };
}  // namespace std