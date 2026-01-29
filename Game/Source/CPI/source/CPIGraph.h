#pragma once

#include "Tasks/CPITask.h"

#include <wiAllocator.h>
#include <unordered_map>

namespace Giperion
{
    namespace CPI
    {
        class CPIGraph_Node;
        class CPIGraph;

        using CPIGraphNodePtr = wi::allocator::shared_ptr<CPIGraph_Node>;
        using CPIGraphNodeWeakPtr = wi::allocator::weak_ptr<CPIGraph_Node>;

        class CPIGraph_Node
        {
            friend CPIGraph;

        public:
            CPIGraph_Node()
                : m_task(nullptr)
            {
            }
            CPIGraph_Node(CPITaskHandle task)
                : m_task(task)
            {
            }

            CPITaskHandle GetTask() const { return m_task; }

            const std::vector<CPIGraphNodePtr>& GetChildNodes() const
            {
                return m_nextNodes;
            }
            const std::vector<CPIGraphNodePtr>& GetParentNodes() const
            {
                return m_prevNodes;
            }

        public:
            std::vector<CPIGraphNodePtr> m_prevNodes;
            std::vector<CPIGraphNodePtr> m_nextNodes;

            CPITaskHandle m_task;
        };

        enum class EGraphState : uint8_t
        {
            UpToDate,
            MustBeRecompile,
        };

        class CPIGraph
        {
        public:
            CPIGraph();

            bool AddTask(CPITaskHandle task);

            bool Compile();

            CPIGraphNodeWeakPtr GetRootNode() const { return m_rootNode; }
            CPIGraphNodeWeakPtr GetNode(CPITaskHandle handle);

            void MarkGraphDirty() { m_state = EGraphState::MustBeRecompile; }
            EGraphState GetState() { return m_state; }

        private:
            CPITaskHandle CreateTaskHandle(const CPITaskPtr& task);
            CPIGraphNodePtr m_rootNode;
            std::unordered_map<CPITaskHandle, CPIGraphNodePtr> m_nodesByHandle;

            wi::vector<CPITaskHandle> m_tasksToCompile;

            // Tasks that have prerequisites but cannot currently reference them
            wi::vector<CPITaskHandle> m_abandonedTasks;

            EGraphState m_state;
        };
    }  // namespace CPI

}  // namespace Giperion