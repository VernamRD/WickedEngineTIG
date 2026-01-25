#include "CPIGraph.h"
#include <iterator>
#include <ranges>

namespace Giperion
{
    namespace GPUCPI
    {
        CPIGraph::CPIGraph()
        {
            auto rootTask = CPITask::CreateTask<CPITask>("Root");
            m_rootNode = wi::allocator::make_shared<CPIGraph_Node>(rootTask);
        }

        bool CPIGraph::AddTask(CPITaskHandle task)
        {
            m_tasksToCompile.push_back(task);
            return true;
        }

        bool CPIGraph::Compile()
        {
            if (m_tasksToCompile.empty()) return true;

            std::vector<CPIGraphNodePtr> nodesWithPrerequisite;

            CPITaskHandle task = m_tasksToCompile[m_tasksToCompile.size() - 1];

            while (task.IsValid())
            {
                CPIGraphNodePtr newNode = wi::allocator::make_shared<CPIGraph_Node>(task);
                m_nodesByHandle.insert({task, newNode});

                if (task.GetPrerequisites().empty())
                {
                    m_rootNode->m_nextNodes.push_back(newNode);
                    newNode->m_prevNodes.push_back(m_rootNode);
                }
                else
                {
                    nodesWithPrerequisite.push_back(newNode);
                }

                m_tasksToCompile.pop_back();

                if (m_tasksToCompile.empty()) break;
                task = m_tasksToCompile[m_tasksToCompile.size() - 1];
            }

            m_tasksToCompile.clear();

            // Try resolve abondened tasks
            if (m_abandonedTasks.size() > 0)
            {
                for (auto abandonedTask : m_abandonedTasks)
                {
                    auto nodePtr = m_nodesByHandle[abandonedTask];
                    if (nodePtr.IsValid())
                    {
                        nodesWithPrerequisite.push_back(nodePtr);
                    }
                }

                m_abandonedTasks.clear();
            }

            // Coonect prerequisite
            for (auto nodeWithPrerequisite : nodesWithPrerequisite)
            {
                bool bAllPrerequisiteExist =
                    std::ranges::all_of(nodeWithPrerequisite->GetTask().GetPrerequisites(),
                        [this](const auto& key) { return m_nodesByHandle.contains(key); });

                if (bAllPrerequisiteExist)
                {
                    for (auto prerequisiteTask : nodeWithPrerequisite->GetTask().GetPrerequisites())
                    {
                        auto parentNode = m_nodesByHandle[prerequisiteTask];
                        parentNode->m_nextNodes.push_back(nodeWithPrerequisite);
                        nodeWithPrerequisite->m_prevNodes.push_back(parentNode);
                    }
                }
                // Defer the task for future compilations. Current requirements are unattainable.
                else
                {
                    m_abandonedTasks.push_back(nodeWithPrerequisite->GetTask());
                }
            }

            return true;
        }

        CPIGraphNodeWeakPtr CPIGraph::GetNode(CPITaskHandle handle)
        {
            return m_nodesByHandle.contains(handle) ? m_nodesByHandle[handle]
                                                    : CPIGraphNodeWeakPtr{};
        }
    }  // namespace GPUCPI
}  // namespace Giperion
