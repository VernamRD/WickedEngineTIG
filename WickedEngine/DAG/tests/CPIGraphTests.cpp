#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "CPI.h"
#include "CPIGraph.h"

using namespace Giperion::CPI;

template <typename GetNodesFunc>
testing::AssertionResult Contains(CPIGraphNodePtr inNode,
    const std::vector<CPIGraphNodePtr>& testNodes, GetNodesFunc getNodes,
    const std::string& nodesType, bool bContainsAll, bool bInvert)
{
    EXPECT_THAT(inNode, testing::NotNull());

    std::vector<CPIGraphNodePtr> nodes = getNodes(inNode);
    std::set<CPIGraphNodePtr> nodesSet(nodes.begin(), nodes.end());
    std::set<CPIGraphNodePtr> testNodesSet(testNodes.begin(), testNodes.end());

    std::vector<CPIGraphNodePtr> result;
    std::set_intersection(nodesSet.begin(), nodesSet.end(), testNodesSet.begin(),
        testNodesSet.end(), std::back_inserter(result));

    auto bContains = bContainsAll ? result == testNodes : result.size() > 0;
    if (bInvert) bContains = !bContains;

    if (bContains)
    {
        return testing::AssertionSuccess();
    }

    std::string strRealNodes;
    for (auto node : nodes)
    {
        strRealNodes += node->GetTask().GetName() + "\t";
    }

    std::string strExpectedNodes;
    for (auto node : testNodes)
    {
        strExpectedNodes += node->GetTask().GetName() + "\t";
    }

    std::string expectPhrase =
        bContainsAll
        ? (bInvert ? "contains all" : "does not contains all")
        : (bInvert ? "contains any" : "does not contains any");

    return testing::AssertionFailure()
           << inNode->GetTask().GetName() << " node with " << nodesType
           << " nodes: " << strRealNodes << " " << expectPhrase
           << " expected nodes: " << strExpectedNodes;
}

testing::AssertionResult ContainsParentAll(
    CPIGraphNodePtr inNode, const std::vector<CPIGraphNodePtr>& testNodes)
{
    return Contains(inNode, testNodes, [](const auto& node){return node->GetParentNodes();}, "Parent", true, false);
}

testing::AssertionResult NotContainsParentAll(
    CPIGraphNodePtr inNode, const std::vector<CPIGraphNodePtr>& testNodes)
{
    return Contains(
        inNode, testNodes, [](const auto& node) { return node->GetParentNodes(); }, "Parent", true,
        true);
}

testing::AssertionResult ContainsParentAny(
    CPIGraphNodePtr inNode, const std::vector<CPIGraphNodePtr>& testNodes)
{
    return Contains(
        inNode, testNodes, [](const auto& node) { return node->GetParentNodes(); }, "Parent", false,
        false);
}

testing::AssertionResult NotContainsParentAny(
    CPIGraphNodePtr inNode, const std::vector<CPIGraphNodePtr>& testNodes)
{
    return Contains(
        inNode, testNodes, [](const auto& node) { return node->GetParentNodes(); }, "Parent", false,
        true);
}

testing::AssertionResult ContainsChildAll(
    CPIGraphNodePtr inNode, const std::vector<CPIGraphNodePtr>& testNodes)
{
    return Contains(
        inNode, testNodes, [](const auto& node) { return node->GetChildNodes(); }, "Child", true,
        false);
}

testing::AssertionResult NotContainsChildAll(
    CPIGraphNodePtr inNode, const std::vector<CPIGraphNodePtr>& testNodes)
{
    return Contains(
        inNode, testNodes, [](const auto& node) { return node->GetChildNodes(); }, "Child", true,
        true);
}

testing::AssertionResult ContainsChildAny(
    CPIGraphNodePtr inNode, const std::vector<CPIGraphNodePtr>& testNodes)
{
    return Contains(
        inNode, testNodes, [](const auto& node) { return node->GetChildNodes(); }, "Child", false,
        false);
}

testing::AssertionResult NotContainsChildAny(
    CPIGraphNodePtr inNode, const std::vector<CPIGraphNodePtr>& testNodes)
{
    return Contains(
        inNode, testNodes, [](const auto& node) { return node->GetChildNodes(); }, "Child", false,
        true);
}

TEST(CPIGraph, SimpleAdd)
{
    auto task1 = TaskFactory::CreateTask<CPUTask>("task1");
    auto task2 = TaskFactory::CreateTask<CPUTask>("task2");
    auto task3 = TaskFactory::CreateTask<CPUTask>("task3");

    CPIGraph graph;
    graph.AddTask(task1);
    graph.AddTask(task2);
    graph.AddTask(task3);

    graph.Compile();

    auto rootNode = graph.GetRootNode().lock();
    auto task1node = graph.GetNode(task1).lock();
    auto task2node = graph.GetNode(task2).lock();
    auto task3node = graph.GetNode(task3).lock();

    EXPECT_TRUE(NotContainsParentAny(rootNode, {rootNode, task1node, task2node, task3node}));
    EXPECT_TRUE(NotContainsChildAny(rootNode, {rootNode}));
    EXPECT_TRUE(ContainsChildAll(rootNode, {task1node, task2node, task3node}));

    EXPECT_TRUE(ContainsParentAll(task1node, {rootNode}));
    EXPECT_TRUE(NotContainsParentAny(task1node, {task1node, task2node, task3node}));
    EXPECT_TRUE(NotContainsChildAny(task1node, {task1node, rootNode, task2node, task3node}));

    EXPECT_TRUE(ContainsParentAll(task2node, {rootNode}));
    EXPECT_TRUE(NotContainsParentAny(task2node, {task2node, task1node, task3node}));
    EXPECT_TRUE(NotContainsChildAny(task2node, {task2node, rootNode, task1node, task3node}));

    EXPECT_TRUE(ContainsParentAll(task3node, {rootNode}));
    EXPECT_TRUE(NotContainsParentAny(task3node, {task3node, task1node, task2node}));
    EXPECT_TRUE(NotContainsChildAny(task3node, {task3node, rootNode, task1node, task2node}));
}

TEST(CPIGraph, TwoPrerequisitedNodes)
{
    auto task1 = TaskFactory::CreateTask<CPUTask>("task1");
    auto task2 = TaskFactory::CreateTask<CPUTask>("task2", {task1});
    auto task3 = TaskFactory::CreateTask<CPUTask>("task3", {task1});

    CPIGraph graph;
    graph.AddTask(task1);
    graph.AddTask(task2);
    graph.AddTask(task3);

    graph.Compile();

    auto rootNode = graph.GetRootNode().lock();
    auto task1node = graph.GetNode(task1).lock();
    auto task2node = graph.GetNode(task2).lock();
    auto task3node = graph.GetNode(task3).lock();

    EXPECT_TRUE(ContainsParentAll(task1node, {rootNode}));
    EXPECT_TRUE(NotContainsParentAny(task1node, {task1node, task2node, task3node}));
    EXPECT_TRUE(NotContainsChildAny(task1node, {task1node, rootNode}));
    EXPECT_TRUE(ContainsChildAll(task1node, {task2node, task3node}));

    EXPECT_TRUE(ContainsParentAll(task2node, {task1node}));
    EXPECT_TRUE(NotContainsParentAny(task2node, {rootNode, task2node, task3node}));
    EXPECT_TRUE(NotContainsChildAny(task2node, {rootNode, task1node, task2node, task3node}));

    EXPECT_TRUE(ContainsParentAll(task3node, {task1node}));
    EXPECT_TRUE(NotContainsParentAny(task3node, {rootNode, task3node, task2node}));
    EXPECT_TRUE(NotContainsChildAny(task3node, {rootNode, task1node, task2node, task3node}));
}

TEST(CPIGraph, DoublePrerequisite)
{
    auto task1 = TaskFactory::CreateTask<CPUTask>("task1");
    auto task2 = TaskFactory::CreateTask<CPUTask>("task2");
    auto task3 = TaskFactory::CreateTask<CPUTask>("task3", {task1, task2});

    CPIGraph graph;
    graph.AddTask(task1);
    graph.AddTask(task2);
    graph.AddTask(task3);

    graph.Compile();

    auto rootNode = graph.GetRootNode().lock();
    auto task1node = graph.GetNode(task1).lock();
    auto task2node = graph.GetNode(task2).lock();
    auto task3node = graph.GetNode(task3).lock();

    EXPECT_TRUE(ContainsParentAll(task1node, {rootNode}));
    EXPECT_TRUE(NotContainsParentAny(task1node, {task1node, task2node, task3node}));
    EXPECT_TRUE(NotContainsChildAny(task1node, {rootNode, task1node, task2node}));
    EXPECT_TRUE(ContainsChildAll(task1node, {task3node}));

    EXPECT_TRUE(ContainsParentAll(task2node, {rootNode}));
    EXPECT_TRUE(NotContainsParentAny(task2node, {task1node, task2node, task3node}));
    EXPECT_TRUE(NotContainsChildAny(task2node, {rootNode, task1node, task2node}));
    EXPECT_TRUE(ContainsChildAll(task2node, {task3node}));

    EXPECT_TRUE(ContainsParentAll(task3node, {task1node, task2node}));
    EXPECT_TRUE(NotContainsParentAny(task3node, {rootNode, task3node}));
    EXPECT_TRUE(NotContainsChildAny(task3node, {rootNode, task1node, task2node, task3node}));
}

TEST(CPIGraph, Hierarchy)
{
    auto task1 = TaskFactory::CreateTask<CPUTask>("task1");
    auto task2 = TaskFactory::CreateTask<CPUTask>("task2");
    auto task3 = TaskFactory::CreateTask<CPUTask>("task3", {task2});

    CPIGraph graph;
    graph.AddTask(task1);
    graph.AddTask(task2);
    graph.AddTask(task3);

    graph.Compile();

    auto rootNode = graph.GetRootNode().lock();
    auto task1node = graph.GetNode(task1).lock();
    auto task2node = graph.GetNode(task2).lock();
    auto task3node = graph.GetNode(task3).lock();

    EXPECT_TRUE(ContainsParentAll(task1node, {rootNode}));
    EXPECT_TRUE(NotContainsParentAny(task1node, {task2node, task3node}));
    EXPECT_TRUE(NotContainsChildAny(task1node, {rootNode, task2node, task3node}));

    EXPECT_TRUE(ContainsParentAll(task2node, {rootNode}));
    EXPECT_TRUE(NotContainsParentAny(task2node, {task1node, task3node}));
    EXPECT_TRUE(NotContainsChildAny(task2node, {rootNode, task1node}));
    EXPECT_TRUE(ContainsChildAll(task2node, {task3node}));

    EXPECT_TRUE(NotContainsParentAny(task3node, {rootNode, task1node}));
    EXPECT_TRUE(NotContainsChildAny(task3node, {rootNode, task1node, task2node}));
    EXPECT_TRUE(ContainsParentAll(task3node, {task2node}));
}

TEST(CPIGraph, Hierarchy2)
{
    auto task1 = TaskFactory::CreateTask<CPUTask>("task1");
    auto task2 = TaskFactory::CreateTask<CPUTask>("task2", {task1});
    auto task3 = TaskFactory::CreateTask<CPUTask>("task3", {task2});

    CPIGraph graph;
    graph.AddTask(task1);
    graph.AddTask(task2);
    graph.AddTask(task3);

    graph.Compile();

    auto rootNode = graph.GetRootNode().lock();
    auto task1node = graph.GetNode(task1).lock();
    auto task2node = graph.GetNode(task2).lock();
    auto task3node = graph.GetNode(task3).lock();

    EXPECT_TRUE(ContainsParentAll(task1node, {rootNode}));
    EXPECT_TRUE(NotContainsParentAny(task1node, {task2node, task3node}));
    EXPECT_TRUE(NotContainsChildAny(task1node, {rootNode, task3node}));
    EXPECT_TRUE(ContainsChildAll(task1node, {task2node}));

    EXPECT_TRUE(ContainsParentAll(task2node, {task1node}));
    EXPECT_TRUE(NotContainsParentAny(task2node, {rootNode, task3node}));
    EXPECT_TRUE(ContainsChildAll(task2node, {task3node}));
    EXPECT_TRUE(NotContainsChildAny(task2node, {rootNode, task1node}));

    EXPECT_TRUE(NotContainsParentAny(task3node, {rootNode, task1node}));
    EXPECT_TRUE(NotContainsChildAny(task3node, {rootNode, task1node, task2node}));
    EXPECT_TRUE(ContainsParentAll(task3node, {task2node}));
}

TEST(CPIGraph, Abandoned)
{
    auto task1 = TaskFactory::CreateTask<CPUTask>("task1");
    auto abandonedTask1 = TaskFactory::CreateTask<CPUTask>("abandonedTask1", {task1});
    auto abandonedTask2 = TaskFactory::CreateTask<CPUTask>("abandonedTask2", {task1});

    CPIGraph graph;
    graph.AddTask(abandonedTask1);
    graph.AddTask(abandonedTask2);

    graph.Compile();

    auto rootNode = graph.GetRootNode().lock();
    auto abandonedTaskNode1 = graph.GetNode(abandonedTask1).lock();
    auto abandonedTaskNode2 = graph.GetNode(abandonedTask2).lock();

    EXPECT_THAT(graph.GetNode(task1).lock(), testing::IsNull());
    EXPECT_TRUE(NotContainsChildAny(rootNode, {abandonedTaskNode1, abandonedTaskNode2}));

    EXPECT_TRUE(NotContainsParentAny(abandonedTaskNode1, {rootNode, abandonedTaskNode2}));
    EXPECT_TRUE(NotContainsChildAny(abandonedTaskNode1, {rootNode, abandonedTaskNode2}));

    EXPECT_TRUE(NotContainsParentAny(abandonedTaskNode2, {rootNode, abandonedTaskNode1}));
    EXPECT_TRUE(NotContainsChildAny(abandonedTaskNode2, {rootNode, abandonedTaskNode1}));

    graph.AddTask(task1);
    graph.Compile();

    auto task1node = graph.GetNode(task1).lock();

    EXPECT_TRUE(ContainsParentAll(task1node, {rootNode}));
    EXPECT_TRUE(NotContainsParentAny(task1node, {task1node, abandonedTaskNode1, abandonedTaskNode2}));
    EXPECT_TRUE(NotContainsChildAny(task1node, {rootNode}));
    EXPECT_TRUE(ContainsChildAll(task1node, {abandonedTaskNode1, abandonedTaskNode2}));

    EXPECT_TRUE(ContainsParentAll(abandonedTaskNode1, {task1node}));
    EXPECT_TRUE(NotContainsParentAny(abandonedTaskNode1, {rootNode, abandonedTaskNode2}));
    EXPECT_TRUE(NotContainsChildAny(abandonedTaskNode1, {rootNode, abandonedTaskNode1, abandonedTaskNode2}));

    EXPECT_TRUE(ContainsParentAll(abandonedTaskNode2, {task1node}));
    EXPECT_TRUE(NotContainsParentAny(abandonedTaskNode2, {rootNode, abandonedTaskNode1, abandonedTaskNode2}));
    EXPECT_TRUE(NotContainsChildAny(abandonedTaskNode2, {rootNode, task1node, abandonedTaskNode1, abandonedTaskNode2}));
}