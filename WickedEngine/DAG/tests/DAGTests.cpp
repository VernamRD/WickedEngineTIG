#include "gtest/gtest.h"

#include "DAG.h"
#include "Tasks/TaskFactory.h"

using namespace wi::compute;

TEST(DAG, OneLayer)
{
    auto task1 = TaskFactory::CreateTask<CPUTask>("task1");
    auto task2 = TaskFactory::CreateTask<CPUTask>("task2");
    auto task3 = TaskFactory::CreateTask<CPUTask>("task3");
    
    DAG dag;
    dag.AddTask(task1);
    dag.AddTask(task2);
    dag.AddTask(task3);

    EXPECT_TRUE(dag.Compile());
}

TEST(DAG, OneLayer_Big)
{
    auto task1 = TaskFactory::CreateTask<CPUTask>("task1");
    auto task2 = TaskFactory::CreateTask<CPUTask>("task2");
    auto task3 = TaskFactory::CreateTask<CPUTask>("task3");
    auto task4 = TaskFactory::CreateTask<CPUTask>("task4");
    auto task5 = TaskFactory::CreateTask<CPUTask>("task5");
    
    DAG dag;
    dag.AddTasks({task1, task2, task3, task4, task5});
    
    EXPECT_TRUE(dag.Compile());
}

TEST(DAG, TwoLayers)
{
    auto task1 = TaskFactory::CreateTask<CPUTask>("task1");
    auto task2 = TaskFactory::CreateTask<CPUTask>("task2", {task1});
    auto task3 = TaskFactory::CreateTask<CPUTask>("task3", {task1});
    
    DAG dag;
    dag.AddTasks({task1, task2, task3});
    
    EXPECT_TRUE(dag.Compile());
}

TEST(DAG, TwoLayers_DoublePrerequisites)
{
    auto task1 = TaskFactory::CreateTask<CPUTask>("task1");
    auto task2 = TaskFactory::CreateTask<CPUTask>("task2");
    auto task3 = TaskFactory::CreateTask<CPUTask>("task3", {task1, task2});
    
    DAG dag;
    dag.AddTasks({task1, task2, task3});
    
    EXPECT_TRUE(dag.Compile());
}

TEST(DAG, ThreeLayers)
{
    auto task1 = TaskFactory::CreateTask<CPUTask>("task1");
    auto task2 = TaskFactory::CreateTask<CPUTask>("task2", {task1});
    auto task3 = TaskFactory::CreateTask<CPUTask>("task3", {task2});
    
    DAG dag;
    dag.AddTasks({task1, task2, task3});
    
    EXPECT_TRUE(dag.Compile());
}

TEST(DAG, ThreeLayers_Complex1)
{
    auto task1 = TaskFactory::CreateTask<CPUTask>("task1");
    auto task2 = TaskFactory::CreateTask<CPUTask>("task2", {task1});
    auto task3 = TaskFactory::CreateTask<CPUTask>("task3", {task2});
    auto task4 = TaskFactory::CreateTask<CPUTask>("task4", {task2});
    auto task5 = TaskFactory::CreateTask<CPUTask>("task5", {task2});
    
    DAG dag;
    dag.AddTasks({task1, task2, task3, task4, task5});
    
    EXPECT_TRUE(dag.Compile());
}

TEST(DAG, ThreeLayers_Complex2)
{
    auto task1 = TaskFactory::CreateTask<CPUTask>("task1");
    auto task2 = TaskFactory::CreateTask<CPUTask>("task2", {task1});
    auto task3 = TaskFactory::CreateTask<CPUTask>("task3", {task2});
    auto task4 = TaskFactory::CreateTask<CPUTask>("task4", {task2});
    auto task5 = TaskFactory::CreateTask<CPUTask>("task5", {task1});
    
    DAG dag;
    dag.AddTasks({task1, task2, task3, task4, task5});
    
    EXPECT_TRUE(dag.Compile());
}

TEST(DAG, ThreeLayers_Complex3)
{
    auto task1 = TaskFactory::CreateTask<CPUTask>("task1");
    auto task2 = TaskFactory::CreateTask<CPUTask>("task2", {task1});
    auto task3 = TaskFactory::CreateTask<CPUTask>("task3", {task1});
    auto task4 = TaskFactory::CreateTask<CPUTask>("task4", {task3});
    auto task5 = TaskFactory::CreateTask<CPUTask>("task5", {task1});
    
    DAG dag;
    dag.AddTasks({task1, task2, task3, task4, task5});
    
    EXPECT_TRUE(dag.Compile());
}