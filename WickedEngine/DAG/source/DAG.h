#pragma once

#include "Tasks/Task.h"

#include <wiAllocator.h>
#include <unordered_map>
#include <utility>
#include <unordered_set>

namespace wi::compute
{
    class DAG_Node;
    class DAG;

    using DAGNodePtr = std::shared_ptr<DAG_Node>;
    using DAGNodeWeakPtr = std::weak_ptr<DAG_Node>;

    class DAG_Node
    {
        friend DAG;

    public:
        DAG_Node()
            : m_task(nullptr)
        {
        }
        DAG_Node(TaskHandle task)
            : m_task(std::move(task))
        {
        }

        [[nodiscard]] TaskHandle GetTask() const { return m_task; }

    public:
        
        TaskHandle m_task;
    };

    enum class EDAGState : uint32_t
    {
        UpToDate,
        MustBeRecompile,
    };

    class DAG
    {
    public:
        DAG();

        void Initialize();
        bool AddTask(TaskHandle task);
        bool AddTasks(std::initializer_list<TaskHandle> taskList);
        
        bool Compile();
        
        DAGNodePtr GetNode(const TaskHandle& handle);
        const DAGNodePtr GetNode(const TaskHandle& handle) const;

        void MarkGraphDirty() { m_state = EDAGState::MustBeRecompile; }
        EDAGState GetState() { return m_state; }
        
        [[nodiscard]] std::string DumpExecutionLayersToDot() const;

    private:
        // All nodes
        std::unordered_map<TaskHandle, DAGNodePtr> m_nodesByHandle;
        
        std::vector<std::vector<DAGNodePtr>> executionLayers;
        std::unordered_set<TaskHandle> m_tasksToCompile;

        // Tasks that have prerequisites but cannot currently reference them
        std::unordered_set<DAGNodePtr> m_abandonedNodes;

        EDAGState m_state;
    };
}  // namespace wi::compute
