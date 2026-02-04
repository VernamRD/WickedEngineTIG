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

    using DAGNodePtr = std::unique_ptr<DAG_Node>;
    using DAGNodeRawPtr = DAG_Node*;

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
    
    struct DAG_Layer
    {
        std::vector<DAGNodeRawPtr> nodes; 
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
        bool AddTask(const TaskHandle& task);
        bool AddTasks(std::initializer_list<TaskHandle> taskList);
        
        bool Compile();
        
        DAGNodeRawPtr GetNode(const TaskHandle& handle);
        DAGNodeRawPtr GetNode(const TaskHandle& handle) const;

        void MarkGraphDirty() { m_state = EDAGState::MustBeRecompile; }
        EDAGState GetState() { return m_state; }
        
        [[nodiscard]] std::string DumpExecutionLayersToDot() const;

    private:
        // All nodes
        std::unordered_map<TaskHandle, DAGNodePtr> m_nodesByHandle;
        
        std::vector<DAG_Layer> executionLayers;
        std::unordered_set<TaskHandle> m_tasksToCompile;

        // Tasks that have prerequisites but cannot currently reference them
        std::unordered_set<TaskHandle> m_abandonedTasks;

        EDAGState m_state;
    };
}  // namespace wi::compute
