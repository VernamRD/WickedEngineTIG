#include "DAG.h"

#include "Tasks/TaskFactory.h"

#include "wiGraphicsDevice_Vulkan.h"

#include <algorithm>
#include <iterator>
#include <ranges>

namespace Giperion
{
    namespace Compute
    {
        DAG::DAG()
        {
            auto rootTask = TaskFactory::CreateTask<Task>("Root");
            m_rootNode = wi::allocator::make_shared<DAG_Node>(rootTask);
        }

        void DAG::Initialize()
        {
            auto device_Vk = static_cast<wi::graphics::GraphicsDevice_Vulkan*>(wi::graphics::GetDevice());
            auto device = device_Vk->GetDevice();
            VkCommandPoolCreateInfo poolInfo{};
            poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
            poolInfo.queueFamilyIndex = device_Vk->GetGraphicsFamilyIndex(); 
            poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
            
            VkCommandPool primaryPool;
            vkCreateCommandPool(device, &poolInfo, nullptr, &primaryPool);
            
            VkCommandPool secondaryPool;
            vkCreateCommandPool(device, &poolInfo, nullptr, &secondaryPool);
            
            
            
            
            // Create primary cmd pool and cmd
            {
                VkCommandBufferAllocateInfo allocInfo = {};
                allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
                allocInfo.commandPool = primaryPool;
                allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
                allocInfo.commandBufferCount = 1;
            
                VkCommandBuffer primaryCmd;
                vkAllocateCommandBuffers(device, &allocInfo, &primaryCmd);    
            }
            
            // Create secondary cmd pool and cmd
            {
                const uint32_t SECONDARY_COUNT = 64;
            
                VkCommandBufferAllocateInfo allocInfo = {};
                allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
                allocInfo.commandPool = secondaryPool;
                allocInfo.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
                allocInfo.commandBufferCount = SECONDARY_COUNT;
                
                VkCommandBuffer secondaryCmds[SECONDARY_COUNT];
                vkAllocateCommandBuffers(device, &allocInfo, secondaryCmds);
            }
        }

        bool DAG::AddTask(TaskHandle task)
        {
            m_tasksToCompile.push_back(task);
            MarkGraphDirty();

            return true;
        }

        bool DAG::Compile()
        {
            if (m_state != EDAGState::MustBeRecompile && m_tasksToCompile.empty()) return true;

            std::vector<DAGNodePtr> nodesWithPrerequisite;

            TaskHandle task = m_tasksToCompile[m_tasksToCompile.size() - 1];

            while (task.IsValid())
            {
                DAGNodePtr newNode = wi::allocator::make_shared<DAG_Node>(task);
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

            m_state = EDAGState::UpToDate;

            return true;
        }

        DAGNodeWeakPtr DAG::GetNode(TaskHandle handle)
        {
            return m_nodesByHandle.contains(handle) ? m_nodesByHandle[handle]
                                                    : DAGNodeWeakPtr{};
        }
    }  // namespace Compute
}  // namespace Giperion