#pragma once

#include <wiAllocator.h>
#include <type_traits>
#include <string>

namespace Giperion
{
    namespace Compute
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

            const std::string& GetName() const;
            const PrerequisitesType& GetPrerequisites() const;

            bool IsValid() const { return m_handle != nullptr; }

            bool operator==(const TaskHandle&) const = default;
            inline size_t hash_value() const
            {
                return std::hash<Task*>{}(m_handle.get());
            }

        private:
            TaskPtr GetTaskPtr() const;

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
            virtual ~Task() = default;
            
            const PrerequisitesType& GetPrerequisites() const;
            const std::string& GetName() const { return m_name; }
            ETaskType GetTaskType() const { return m_taskType; }

        protected:
            Task() = delete;
            Task(std::string&& inName);
            Task(std::string&& inName, const wi::vector<TaskHandle>& prerequisites);
            Task(std::string&& inName, std::initializer_list<TaskHandle> prerequisites);

        private:
            TaskHandle InitHandle(TaskPtr thisPtr) const;
            
            std::string m_name;
            PrerequisitesType m_prerequisites;
            ETaskType m_taskType = ETaskType::Undefined;
        };

    }  // namespace Compute

}  // namespace Giperion

namespace std
{
    template <>
    struct hash<Giperion::Compute::TaskHandle>
    {
        size_t operator()(const Giperion::Compute::TaskHandle& handle) const noexcept
        {
            return handle.hash_value();
        }
    };
}  // namespace std