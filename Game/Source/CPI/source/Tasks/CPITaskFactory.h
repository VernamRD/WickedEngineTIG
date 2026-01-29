#pragma once

#include "CPICPUTask.h"
#include "CPIGPUTask.h"

namespace Giperion
{
    namespace CPI
    {
        template <typename TaskType>
        concept IsGPUTask = std::derived_from<TaskType, GPUTask>;
        template <typename TaskType>
        concept IsCPUTask = std::derived_from<TaskType, CPUTask>;
        template <typename TaskType>
        concept IsUndefinedTask =
            (std::is_same_v<TaskType, CPITask> || std::derived_from<TaskType, CPITask>) && !IsCPUTask<TaskType> && !IsGPUTask<TaskType>;

        class TaskFactory
        {
        public:
            template <typename TaskType, typename... Args>
                requires IsGPUTask<TaskType>
            static CPITaskHandle CreateTask(std::string&& taskName, Args&&... args)
            {
                auto task = std::shared_ptr<TaskType>(new TaskType(std::move(taskName), {}, std::forward<Args>(args)...));
                task->m_taskType = ETaskType::GPU;
                return task->InitHandle(task);
            }

            template <typename TaskType, typename... Args>
                requires IsGPUTask<TaskType>
            static CPITaskHandle CreateTask(std::string&& taskName, std::initializer_list<CPITaskHandle> prerequisites, Args&&... args)
            {
                auto task = std::shared_ptr<TaskType>(new TaskType(std::move(taskName), prerequisites, std::forward<Args>(args)...));
                task->m_taskType = ETaskType::GPU;
                return task->InitHandle(task);
            }

            template <typename TaskType, typename... Args>
                requires IsCPUTask<TaskType>
            static CPITaskHandle CreateTask(std::string&& taskName, Args&&... args)
            {
                auto task = std::shared_ptr<TaskType>(new TaskType(std::move(taskName), {}, std::forward<Args>(args)...));
                task->m_taskType = ETaskType::CPU;
                return task->InitHandle(task);
            }

            template <typename TaskType, typename... Args>
                requires IsCPUTask<TaskType>
            static CPITaskHandle CreateTask(std::string&& taskName, std::initializer_list<CPITaskHandle> prerequisites, Args&&... args)
            {
                auto task = std::shared_ptr<TaskType>(new TaskType(std::move(taskName), prerequisites, std::forward<Args>(args)...));
                task->m_taskType = ETaskType::CPU;
                return task->InitHandle(task);
            }

            template <typename TaskType, typename... Args>
                requires IsUndefinedTask<TaskType>
            static CPITaskHandle CreateTask(std::string&& taskName, Args&&... args)
            {
                auto task = std::shared_ptr<TaskType>(new TaskType(std::move(taskName), {}, std::forward<Args>(args)...));
                task->m_taskType = ETaskType::Undefined;
                return task->InitHandle(task);
            }

            template <typename TaskType, typename... Args>
                requires IsUndefinedTask<TaskType>
            static CPITaskHandle CreateTask(std::string&& taskName, std::initializer_list<CPITaskHandle> prerequisites, Args&&... args)
            {
                auto task = std::shared_ptr<TaskType>(new TaskType(std::move(taskName), prerequisites, std::forward<Args>(args)...));
                task->m_taskType = ETaskType::Undefined;
                return task->InitHandle(task);
            }
        };
    }  // namespace CPI
}  // namespace Giperion