#include "DAG.h"

namespace wi
{
    namespace compute
    {

#pragma region ComputeNode
        ComputeNode::ComputeNode(const TaskDesc& desc)
            : m_desc(desc)
        {
        }

        void ComputeNode::AddDependency(ComputeNode* node)
        {
            if (node)
            {
                m_dependencies.push_back(node);
                node->m_dependents.push_back(this);
            }
        }

        bool ComputeNode::IsReady() const
        {
            for (const ComputeNode* dependency : m_dependencies)
            {
                if (dependency->GetState() != NodeState::Completed)
                {
                    return false;
                }
            }

            return true;
        }

        void ComputeNode::RecordCommands(wi::graphics::CommandList cmd)
        {
            if (m_desc.recordFunction)
            {
                m_desc.recordFunction(cmd);
            }
        }
#pragma endregion ComputeNode

#pragma region SecondaryCommandBufferPool

        SecondaryCommandBufferPool::SecondaryCommandBufferPool(wi::graphics::GraphicsDevice* device, uint32_t poolSize)
            : m_device(device)
        {
            auto device_vk = static_cast<wi::graphics::GraphicsDevice_Vulkan*>(m_device);
            m_pool.reserve(poolSize);

            for (uint32_t i = 0; i < poolSize; ++i)
            {
                VkCommandPoolCreateInfo poolInfo = {};
                poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
                poolInfo.queueFamilyIndex = device_vk->GetGraphicsFamilyIndex();
            }            
        }

        SecondaryCommandBufferPool::~SecondaryCommandBufferPool()
        {
            for (wi::graphics::CommandList& cmd : m_pool)
            {
                m_device->DestroyCommandList(cmd);
            }
        }

        wi::graphics::CommandList SecondaryCommandBufferPool::Acquire()
        {
            if (m_available.empty())
            {
                wi::graphics::CommandList cmd = m_device->CreateCommandList(wi::graphics::QUEUE_COMPUTE, true);
                m_pool.push_back(cmd);
                return cmd;
            }
            else
            {
                wi::graphics::CommandList cmd = m_available.front();
                m_available.pop();
                m_inUse.push_back(cmd);
                return cmd;
            }
        }

        void SecondaryCommandBufferPool::Release(wi::graphics::CommandList cmd)
        {
            auto it = std::find(m_inUse.begin(), m_inUse.end(), cmd);
            if (it != m_inUse.end())
            {
                m_inUse.erase(it);
                m_available.push(cmd);
            }
        }

        void SecondaryCommandBufferPool::Reset()
        {
            for (wi::graphics::CommandList& cmd : m_inUse)
            {
                m_available.push(cmd);
            }
            m_inUse.clear();
        }
        
#pragma endregion SecondaryCommandBufferPool
    }  // namespace compute
}  // namespace wi