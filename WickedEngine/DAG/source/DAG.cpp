#include "DAG.h"

#include "Tasks/TaskFactory.h"

#include "wiBacklog.h"
#include "wiGraphicsDevice_Vulkan.h"

#include <algorithm>
#include <iterator>
#include <ranges>
#include <sstream>

namespace wi::compute
{
    struct GraphIterator
    {
    private:
        struct StackFrame
        {
            const DAG_Node& node;
        };
        struct Stack
        {
        public:
            void Push(StackFrame&& newFrame)
            {
                if (deadlockChecker.contains(&newFrame.node))
                {
                    frames.push_back(newFrame);
                    OnDeadlock();
                    return;
                }

                if (visitedNodes.contains(&newFrame.node)) return;

                frames.push_back(newFrame);
                deadlockChecker.insert(&newFrame.node);
                visitedNodes.insert(&newFrame.node);
            }
            StackFrame Pop()
            {
                StackFrame currentFrame = frames.back();
                frames.pop_back();
                deadlockChecker.erase(&currentFrame.node);
                return currentFrame;
            }

            bool IsEmpty() const { return frames.empty(); }

        private:
            void OnDeadlock()
            {
                std::string errorStr;
                for (size_t i = 0; i < frames.size(); ++i)
                {
                    const auto& frame = frames[i];
                    wilog_warning("Frame:%zu %s", i, frame.node.GetTask().GetName().c_str());
                }

                wilog_messagebox("OnDeadlock detected!");
                wi::platform::Exit();
            }

            std::vector<StackFrame> frames;
            std::unordered_set<const DAG_Node*> deadlockChecker;
            std::unordered_set<const DAG_Node*> visitedNodes;
        };

    public:
        GraphIterator() = delete;
        GraphIterator(const DAG& dag)
            : m_dag(dag)
        {
        }

        void IteratePrerequisite(const DAG_Node& node, std::function<void(const DAG_Node&)> onEachNode)
        {
            stack.Push(StackFrame{node});

            while (!stack.IsEmpty())
            {
                StackFrame currentFrame = stack.Pop();
                onEachNode(currentFrame.node);

                for (const auto& prerequisite : currentFrame.node.GetTask().GetPrerequisites())
                {
                    stack.Push(StackFrame{*m_dag.GetNode(prerequisite)});
                }
            }
        }

    private:
        Stack stack;
        const DAG& m_dag;
    };

    DAG::DAG() {}

    void DAG::Initialize()
    {
        //        auto device_Vk = static_cast<wi::graphics::GraphicsDevice_Vulkan*>(wi::graphics::GetDevice());
        //        auto device = device_Vk->GetDevice();
        //        VkCommandPoolCreateInfo poolInfo{};
        //        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        //        poolInfo.queueFamilyIndex = device_Vk->GetGraphicsFamilyIndex();
        //        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        //
        //        VkCommandPool primaryPool;
        //        vkCreateCommandPool(device, &poolInfo, nullptr, &primaryPool);
        //
        //        VkCommandPool secondaryPool;
        //        vkCreateCommandPool(device, &poolInfo, nullptr, &secondaryPool);
        //
        //        // Create primary cmd pool and cmd
        //        {
        //            VkCommandBufferAllocateInfo allocInfo = {};
        //            allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        //            allocInfo.commandPool = primaryPool;
        //            allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        //            allocInfo.commandBufferCount = 1;
        //
        //            VkCommandBuffer primaryCmd;
        //            vkAllocateCommandBuffers(device, &allocInfo, &primaryCmd);
        //        }
        //
        //        // Create secondary cmd pool and cmd
        //        {
        //            const uint32_t SECONDARY_COUNT = 64;
        //
        //            VkCommandBufferAllocateInfo allocInfo = {};
        //            allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        //            allocInfo.commandPool = secondaryPool;
        //            allocInfo.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
        //            allocInfo.commandBufferCount = SECONDARY_COUNT;
        //
        //            VkCommandBuffer secondaryCmds[SECONDARY_COUNT];
        //            vkAllocateCommandBuffers(device, &allocInfo, secondaryCmds);
        //        }
    }

    bool DAG::AddTask(TaskHandle task)
    {
        m_tasksToCompile.insert(task);
        MarkGraphDirty();

        return true;
    }

    bool DAG::AddTasks(std::initializer_list<TaskHandle> taskList)
    {
        m_tasksToCompile.insert(taskList);
        MarkGraphDirty();

        return true;
    }

    bool DAG::Compile()
    {
        if (m_state != EDAGState::MustBeRecompile && m_tasksToCompile.empty()) return true;

        // Nodes set which not processed yet
        std::unordered_set<DAGNodePtr> nodesToCompile;

        for (auto taskToCompile : m_tasksToCompile)
        {
            DAGNodePtr newNode = std::make_shared<DAG_Node>(taskToCompile);
            m_nodesByHandle.insert({taskToCompile, newNode});
            nodesToCompile.insert(newNode);
        }

        // Try recompile abandoned nodes
        nodesToCompile.merge(m_abandonedNodes);
        m_abandonedNodes.clear();

        executionLayers.clear();
        executionLayers.emplace_back();
        std::vector<DAGNodePtr>* executionLayer = &executionLayers.back();
        
        // All nodes contained in any layer
        std::unordered_set<DAGNodePtr> layeredTasks;

        while (executionLayer)
        {
            GraphIterator graphIterator(*this);
            
            std::vector<DAGNodePtr> layerTasks;
            for (const auto& node : nodesToCompile)
            {
                graphIterator.IteratePrerequisite(*node,
                    [this, &layeredTasks, &layerTasks](const DAG_Node& node)
                    {
                        if(layeredTasks.contains(GetNode(node.GetTask()))) return;
                        
                        bool bAllPrerequisitesLayered = true;

                        for (const auto& prerequisite : node.GetTask().GetPrerequisites())
                        {
                            bAllPrerequisitesLayered &= layeredTasks.contains(GetNode(prerequisite));
                        }

                        if (bAllPrerequisitesLayered)
                        {
                            layerTasks.push_back(GetNode(node.GetTask()));
                        }
                    });
            }

            std::vector<DAGNodePtr> layerNodesVec;

            // Add to layer
            layeredTasks.reserve(layeredTasks.size() + layerTasks.size());
            for (const auto& task : layerTasks)
            {
                nodesToCompile.erase(task);
                layerNodesVec.push_back(task);
                layeredTasks.insert(task);
            }

            // Assign layer
            *executionLayer = layerNodesVec;

            if (!nodesToCompile.empty())
            {
                for (auto nodeToCompile : nodesToCompile)
                {
                    bool bAllPrerequisiteExist = std::ranges::all_of(nodeToCompile->GetTask().GetPrerequisites(),
                        [this](const TaskHandle& task) { return GetNode(task) != nullptr && !m_abandonedNodes.contains(GetNode(task)); });

                    if (!bAllPrerequisiteExist)
                    {
                        m_abandonedNodes.insert(nodeToCompile);
                    }
                }

                executionLayers.emplace_back();
                executionLayer = &executionLayers.back();
            }
            else
            {
                executionLayer = nullptr;
            }
        }

        wi::backlog::post(DumpExecutionLayersToDot());

        m_state = EDAGState::UpToDate;

        return true;
    }

    DAGNodePtr DAG::GetNode(const TaskHandle& handle) { return m_nodesByHandle.contains(handle) ? m_nodesByHandle[handle] : nullptr; }

    const DAGNodePtr DAG::GetNode(const TaskHandle& handle) const
    {
        return m_nodesByHandle.contains(handle) ? m_nodesByHandle.find(handle)->second : DAGNodePtr();
    }

    std::string DAG::DumpExecutionLayersToDot() const
    {
        std::ostringstream oss;
        
        oss << "digraph ExecutionLayers {\n";
        oss << "  rankdir=TB;\n";
        oss << "  node [shape=box, style=rounded];\n";
        oss << "  graph [compound=true];\n\n";

        // Якоря для вертикального выравнивания кластеров
        for (size_t i = 0; i < executionLayers.size(); ++i)
        {
            oss << "  layer_anchor_" << i << " [shape=box, width=0.01, label=\"\"];\n";
        }
        oss << "\n";

        for (size_t layerIndex = 0; layerIndex < executionLayers.size(); ++layerIndex)
        {
            oss << "  subgraph cluster_" << layerIndex << " {\n";
            oss << "    label=\"Layer " << layerIndex << "\";\n";
            oss << "    color=lightgrey;\n";
            oss << "    style=rounded;\n";

            for (const DAGNodePtr& node : executionLayers[layerIndex])
            {
                oss << "    \"" << node->GetTask().GetName() << "\";\n";
            }

            // привязываем кластер к якорю
            oss << "    layer_anchor_" << layerIndex << ";\n";
            oss << "  }\n\n";
        }

        // Невидимые рёбра для вертикального порядка слоёв
        for (size_t i = 0; i + 1 < executionLayers.size(); ++i)
        {
            oss << "  layer_anchor_" << i
                << " -> layer_anchor_" << (i + 1)
                << " [style=invis];\n";
        }

        oss << "}\n";
        return oss.str();
    }
}  // namespace wi::compute
