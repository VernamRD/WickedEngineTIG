#pragma once

#include "CPITask.h"
#include <wiAllocator.h>
#include <wiUnorderedMap.h>

namespace Giperion
{
    namespace GPUCPI
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
                : m_task()
            {
            }
            CPIGraph_Node(CPITaskPtr task)
                : m_task(task)
            {
            }

        private:
            wi::vector<CPIGraphNodePtr> m_prevNodes;
            wi::vector<CPIGraphNodePtr> m_nextNodes;

            CPITaskPtr m_task;
        };

        class CPIGraph
        {
        public:
            CPITaskHandle AddTask(CPITaskPtr&& task);

            bool Compile();

        private:
            CPITaskHandle CreateTaskHandle(const CPITaskPtr& task);
            wi::vector<CPIGraphNodePtr> m_nodes;
            wi::unordered_map<CPITaskHandle, CPIGraphNodeWeakPtr> m_nodesByHandle;

            wi::vector<CPITaskPtr> m_tasksToCompile;
        };
    }  // namespace GPUCPI

}  // namespace Giperion