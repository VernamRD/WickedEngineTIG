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

        enum class ETaskType : uint8_t
        {
            Undefined,
            CPU,
            GPU
        };

        class CPITask
        {
            friend class TaskFactory;

        public:
            virtual ~CPITask() = default;
            
            const PrerequisitesType& GetPrerequisites() const;
            const std::string& GetName() const { return m_name; }
            ETaskType GetTaskType() const { return m_taskType; }

        protected:
            CPITask() = delete;
            CPITask(std::string&& inName);
            CPITask(std::string&& inName, const wi::vector<CPITaskHandle>& prerequisites);
            CPITask(std::string&& inName, std::initializer_list<CPITaskHandle> prerequisites);

        private:
            CPITaskHandle InitHandle(CPITaskPtr thisPtr) const;
            
            std::string m_name;
            PrerequisitesType m_prerequisites;
            ETaskType m_taskType = ETaskType::Undefined;
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