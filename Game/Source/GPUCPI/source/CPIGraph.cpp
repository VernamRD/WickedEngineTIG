#include "CPIGraph.h"

namespace Giperion
{
    namespace GPUCPI
    {
        CPITaskHandle CPIGraph::AddTask(CPITaskPtr&& task)
        {
            auto handle = CreateTaskHandle(task);
            m_tasksToCompile.push_back(std::forward<CPITaskPtr>(task));
            return handle;
        }

        bool CPIGraph::Compile()
        {
            for (auto& task : m_tasksToCompile)
            {
                CPIGraph_Node newNode(task);

                for (auto& prerequisite : task->GetPrerequisites())
                {
                    // if(m_nodesByHandle.find(prerequisite)->second)
                    // newNode.m_prevNodes.push_back(prerequisite);
                }
            }

            for (auto& node : m_nodes)
            {
            }

            return true;
        }

        CPITaskHandle CPIGraph::CreateTaskHandle(const CPITaskPtr& task)
        {
            CPITaskHandle handle(task);
            return handle;
        }
    }  // namespace GPUCPI
}  // namespace Giperion
