#pragma once

#include "Tasks/Task.h"

#include <wiAllocator.h>
#include <unordered_map>

namespace Giperion
{
    namespace Compute
    {
        class DAG_Node;
        class DAG;

        using DAGNodePtr = wi::allocator::shared_ptr<DAG_Node>;
        using DAGNodeWeakPtr = wi::allocator::weak_ptr<DAG_Node>;

        class DAG_Node
        {
            friend DAG;

        public:
            DAG_Node()
                : m_task(nullptr)
            {
            }
            DAG_Node(TaskHandle task)
                : m_task(task)
            {
            }

            TaskHandle GetTask() const { return m_task; }

            const std::vector<DAGNodePtr>& GetChildNodes() const
            {
                return m_nextNodes;
            }
            const std::vector<DAGNodePtr>& GetParentNodes() const { return m_prevNodes; }

        public:
            std::vector<DAGNodePtr> m_prevNodes;
            std::vector<DAGNodePtr> m_nextNodes;

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
            bool Compile();

            DAGNodeWeakPtr GetRootNode() const { return m_rootNode; }
            DAGNodeWeakPtr GetNode(TaskHandle handle);

            void MarkGraphDirty() { m_state = EDAGState::MustBeRecompile; }
            EDAGState GetState() { return m_state; }

        private:
            TaskHandle CreateTaskHandle(const TaskPtr& task);
            DAGNodePtr m_rootNode;
            std::unordered_map<TaskHandle, DAGNodePtr> m_nodesByHandle;

            wi::vector<TaskHandle> m_tasksToCompile;

            // Tasks that have prerequisites but cannot currently reference them
            wi::vector<TaskHandle> m_abandonedTasks;

            EDAGState m_state;
        };
    }  // namespace 

}  // namespace Giperion