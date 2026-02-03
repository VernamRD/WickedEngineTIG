#pragma once

#include "CPUTask.h"
#include "GPUTask.h"

namespace wi::compute
{
    template <typename TaskType>
    concept IsGPUTask = std::derived_from<TaskType, GPUTask>;
    template <typename TaskType>
    concept IsCPUTask = std::derived_from<TaskType, CPUTask>;
    template <typename TaskType>
    concept IsUndefinedTask =
        (std::is_same_v<TaskType, Task> || std::derived_from<TaskType, Task>) && !IsCPUTask<TaskType> && !IsGPUTask<TaskType>;

    class TaskFactory
    {
    public:
        template <typename TaskType, typename... Args>
            requires IsGPUTask<TaskType>
        static TaskHandle CreateTask(std::string&& taskName, Args&&... args)
        {
            auto task = std::shared_ptr<TaskType>(new TaskType(std::move(taskName), {}, std::forward<Args>(args)...));
            task->m_taskType = ETaskType::GPU;
            return task->InitHandle(task);
        }

        template <typename TaskType, typename... Args>
            requires IsGPUTask<TaskType>
        static TaskHandle CreateTask(std::string&& taskName, std::initializer_list<TaskHandle> prerequisites, Args&&... args)
        {
            auto task = std::shared_ptr<TaskType>(new TaskType(std::move(taskName), prerequisites, std::forward<Args>(args)...));
            task->m_taskType = ETaskType::GPU;
            return task->InitHandle(task);
        }

        template <typename TaskType, typename... Args>
            requires IsCPUTask<TaskType>
        static TaskHandle CreateTask(std::string&& taskName, Args&&... args)
        {
            auto task = std::shared_ptr<TaskType>(new TaskType(std::move(taskName), {}, std::forward<Args>(args)...));
            task->m_taskType = ETaskType::CPU;
            return task->InitHandle(task);
        }

        template <typename TaskType, typename... Args>
            requires IsCPUTask<TaskType>
        static TaskHandle CreateTask(std::string&& taskName, std::initializer_list<TaskHandle> prerequisites, Args&&... args)
        {
            auto task = std::shared_ptr<TaskType>(new TaskType(std::move(taskName), prerequisites, std::forward<Args>(args)...));
            task->m_taskType = ETaskType::CPU;
            return task->InitHandle(task);
        }

        template <typename TaskType, typename... Args>
            requires IsUndefinedTask<TaskType>
        static TaskHandle CreateTask(std::string&& taskName, Args&&... args)
        {
            auto task = std::shared_ptr<TaskType>(new TaskType(std::move(taskName), {}, std::forward<Args>(args)...));
            task->m_taskType = ETaskType::Undefined;
            return task->InitHandle(task);
        }

        template <typename TaskType, typename... Args>
            requires IsUndefinedTask<TaskType>
        static TaskHandle CreateTask(std::string&& taskName, std::initializer_list<TaskHandle> prerequisites, Args&&... args)
        {
            auto task = std::shared_ptr<TaskType>(new TaskType(std::move(taskName), prerequisites, std::forward<Args>(args)...));
            task->m_taskType = ETaskType::Undefined;
            return task->InitHandle(task);
        }
    };
}  // namespace wi::compute
