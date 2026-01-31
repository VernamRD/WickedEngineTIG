#pragma once

#include <functional>
#include <memory>
#include <string>
#include <vector>
#include <queue>
#include <unordered_map>
#include <mutex>

#include <wiGraphicsDevice_Vulkan.h>

namespace wi
{
    namespace compute
    {
        class ComputeNode;
        class ComputeDAG;

        enum class NodeState
        {
            Pending,
            Sheduled,
            Executing,
            Completed,
        };

        struct NodeExecutionResult
        {
            bool m_success = false;
            uint64_t m_executionTimeNs = 0;
            std::string m_errorMessage;
        };

        struct TaskDesc
        {
            std::string m_name;
            std::function<void(wi::graphics::CommandList)> recordFunction;
            std::vector<std::string> m_dependencies;

            std::vector<const wi::graphics::GPUBuffer*> m_inputBuffers;
            std::vector<const wi::graphics::GPUBuffer*> m_outputBuffers;

            uint32_t m_threadX = 1;
            uint32_t m_threadY = 1;
            uint32_t m_threadZ = 1;
        };

        class ComputeNode
        {
        public:
            ComputeNode(const TaskDesc& desc);

            const std::string& GetName() const { return m_desc.m_name; }
            NodeState GetState() const { return m_state; }

            void AddDependency(ComputeNode* node);
            const std::vector<ComputeNode*>& GetDependencies() const { return m_dependencies; }
            const std::vector<ComputeNode*>& GetDependents() const { return m_dependents; }

            bool IsReady() const;

            void RecordCommands(wi::graphics::CommandList cmd);

            void SetState(NodeState state) { m_state = state; }
            void SetExecutionResult(const NodeExecutionResult& result) { m_result = result; }

            const TaskDesc& GetDesc() const { return m_desc; }

        private:
            TaskDesc m_desc;
            NodeState m_state = NodeState::Pending;

            std::vector<ComputeNode*> m_dependencies;
            std::vector<ComputeNode*> m_dependents;

            NodeExecutionResult m_result;
        };

        class SecondaryCommandBufferPool
        {
        public:
            SecondaryCommandBufferPool(wi::graphics::GraphicsDevice* device, uint32_t poolSize = 32);
            ~SecondaryCommandBufferPool();

            wi::graphics::CommandList Acquire();
            void Release(wi::graphics::CommandList cmd);

            void Reset();

        private:
            wi::graphics::GraphicsDevice* m_device = nullptr;
            std::vector<wi::graphics::CommandList> m_pool;
            std::queue<wi::graphics::CommandList> m_available;
            std::vector<wi::graphics::CommandList> m_inUse;
        };

        struct AsyncComputeSync
        {
            uint64_t m_fenceValule = 0;

            wi::vector<VkSemaphore> m_waitSemaphores;
            wi::vector<VkSemaphore> m_signalSemaphores;
        };

        class ComputeDAG
        {
        public:
            ComputeDAG(wi::graphics::GraphicsDevice* device);
            ~ComputeDAG();

            void AddTask(const TaskDesc& desc);
            bool BuildGraph();

            uint64_t ExecuteAsync();

            bool IsCompleted(uint64_t fenceValue) const;

            void WaitForCompletion(uint64_t fenceValue) const;

            void Clear();

            struct Stats
            {
                uint32_t m_totalNodes = 0;
                uint32_t m_completeNodes = 0;
                uint64_t m_totalExecutionTimeNs = 0;
            };
            Stats GetStats() const;

        private:
            wi::graphics::GraphicsDevice* m_device = nullptr;

            wi::vector<std::unique_ptr<ComputeNode>> m_nodes;
            std::unordered_map<std::string, ComputeNode*> m_nodeMap;

            std::vector<std::vector<ComputeNode*>> m_executionLayers;
            std::unique_ptr<SecondaryCommandBufferPool> m_commandBufferPool;

            wi::graphics::CommandList m_primaryCommandList;

            AsyncComputeSync m_syncState;
            uint64_t m_currentFenceValue = 0;

            void TopologicalSort();
            void ExecuteLayer(const std::vector<ComputeNode*>& layer, std::vector<wi::graphics::CommandList>& secondaryCmds);
            void InsertBarriers(wi::graphics::CommandList cmd, const std::vector<ComputeNode*>& layer);
        };

        class TaskBuilder
        {
        public:
            TaskBuilder(const std::string& name);

            TaskBuilder& SetRecordFunction(std::function<void(wi::graphics::CommandList)> func);
            TaskBuilder& AddDependency(const std::string& taskName);
            TaskBuilder& AddInputBuffer(const wi::graphics::GPUBuffer* buffer);
            TaskBuilder& AddOutputBuffer(const wi::graphics::GPUBuffer* buffer);
            TaskBuilder& SetThreadGroupSize(uint32_t x, uint32_t y = 1, uint32_t z = 1);

            TaskDesc Build() const;

        private:
            TaskDesc m_desc;
        };
    } // namespace compute
} // namespace wi