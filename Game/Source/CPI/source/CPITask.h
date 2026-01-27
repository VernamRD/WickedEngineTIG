#pragma once

#include <wiAllocator.h>
#include <type_traits>

namespace Giperion
{
    namespace CPI
    {
        class CPITask;
        class CPITaskHandle;

        using PrerequisitesType = wi::vector<CPITaskHandle>;
        using CPITaskPtr = std::shared_ptr<CPITask>;

        class CPITaskHandle
        {
        public:
            using HandleType = CPITaskPtr;

            CPITaskHandle() = delete;
            CPITaskHandle(const CPITaskPtr& task);

            const std::string& GetName() const;
            const PrerequisitesType& GetPrerequisites() const;

            bool IsValid() const { return m_handle != nullptr; }

            bool operator==(const CPITaskHandle&) const = default;
            inline size_t hash_value() const
            {
                return std::hash<CPITask*>{}(m_handle.get());
            }

        private:
            CPITaskPtr GetTaskPtr() const;

            HandleType m_handle;
        };

        class CPITask
        {
        public:
            template <typename TaskType>
                requires std::derived_from<TaskType, CPITask>
            static CPITaskHandle CreateTask(std::string&& name)
            {
                auto task = std::shared_ptr<TaskType>(new TaskType(std::move(name)));
                return task->InitHandle(task);
            }

            template <typename TaskType>
                requires std::derived_from<TaskType, CPITask>
            static CPITaskHandle CreateTask(std::string&& name, PrerequisitesType prerequisites)
            {
                auto task = std::shared_ptr<TaskType>(new TaskType(std::move(name), prerequisites));
                return task->InitHandle(task);
            }

            CPITaskHandle InitHandle(CPITaskPtr thisPtr) const;
            const PrerequisitesType& GetPrerequisites() const;
            virtual void Execute() {}

            const std::string& GetName() const { return m_name; }

        private:
            CPITask() = delete;
            CPITask(std::string&& inName);
            CPITask(std::string&& inName, const wi::vector<CPITaskHandle>& prerequisites);

            std::string m_name;
            PrerequisitesType m_prerequisites;
        };

    }  // namespace CPI

}  // namespace Giperion

namespace std
{
    template <>
    struct hash<Giperion::CPI::CPITaskHandle>
    {
        size_t operator()(const Giperion::CPI::CPITaskHandle& handle) const noexcept
        {
            return handle.hash_value();
        }
    };
}  // namespace std