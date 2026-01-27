#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "CPIGraph.h"
#include <WickedEngine.h>
#include <wiGraphicsDevice_Vulkan.h>

using namespace wi::graphics;

// Fixture for initializing GraphicsDevice once for all tests
class CPITasksFixture : public ::testing::Test
{
protected:
    static std::unique_ptr<GraphicsDevice> device;
    static wi::platform::window_type window;

    static void SetUpTestSuite()
    {
        device = std::make_unique<GraphicsDevice_Vulkan>(nullptr, ValidationMode::Verbose, GPUPreference::Discrete);

        wi::graphics::GetDevice() = device.get();

        std::cout << "GraphicsDevice initialized for CPI tests." << std::endl;
    }

    static void TearDownTestSuite()
    {
        if (device)
        {
            device->WaitForGPU();
            device.reset();
        }

        std::cout << "GraphicsDevice destroyed for CPI tests." << std::endl;
    }

    static bool HasFatalFailure() { return device != nullptr; }

    void SetUp() override
    {
        ASSERT_NE(device, nullptr) << "GraphicsDevice does not initialized!";
    }
};

std::unique_ptr<GraphicsDevice> CPITasksFixture::device = nullptr;
wi::platform::window_type CPITasksFixture::window = {};

TEST_F(CPITasksFixture, SubmitComputeCommandListIndependently)
{
    CommandList cmd = device->BeginCommandList_Independent(QUEUE_COMPUTE);
    ASSERT_TRUE(cmd.IsValid());

    GPUBarrier barrier{};
    barrier.type = GPUBarrier::Type::MEMORY;
    device->Barrier(&barrier, 1, cmd);

    auto fence = device->SubmitCommandList_Independent(cmd, "SubmitComputeCommandListIndependently");

    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    EXPECT_TRUE(fence->IsSignaled());
    
    fence->CPUWait();
}

TEST_F(CPITasksFixture, SubmitComputeCommandListWithCPUWait)
{
    CommandList cmd = device->BeginCommandList_Independent(QUEUE_COMPUTE);
    ASSERT_TRUE(cmd.IsValid());

    GPUBarrier barrier{};
    barrier.type = GPUBarrier::Type::MEMORY;
    device->Barrier(&barrier, 1, cmd);

    auto fence = device->SubmitCommandList_Independent(cmd, "SubmitComputeCommandListWithCPUWait");

    fence->CPUWait();
}

TEST_F(CPITasksFixture, SubmitCommandListWithDependencies)
{
    CommandList cmdA = device->BeginCommandList_Independent(QUEUE_COMPUTE);
    ASSERT_TRUE(cmdA.IsValid());
    
    GPUBarrier barrier{};
    barrier.type = GPUBarrier::Type::MEMORY;
    device->Barrier(&barrier, 1, cmdA);

    CommandList cmdB = device->BeginCommandList_Independent(QUEUE_COMPUTE);
    ASSERT_TRUE(cmdB.IsValid());
    
    device->WaitCommandList(cmdB, cmdA);
    
    device->Barrier(&barrier, 1, cmdB);
    
    auto fenceA = device->SubmitCommandList_Independent(cmdA, "SubmitCommandListWithDependencies_A");
    auto fenceB = device->SubmitCommandList_Independent(cmdB, "SubmitCommandListWithDependencies_B");

    fenceB->CPUWait();
    
    EXPECT_TRUE(fenceA->IsSignaled());
    EXPECT_TRUE(fenceB->IsSignaled());
}

TEST_F(CPITasksFixture, SubmitCommandListWithDependenciesMultiple)
{
    CommandList cmdA = device->BeginCommandList_Independent(QUEUE_COMPUTE);
    ASSERT_TRUE(cmdA.IsValid());

    GPUBarrier barrier{};
    barrier.type = GPUBarrier::Type::MEMORY;
    device->Barrier(&barrier, 1, cmdA);

    CommandList cmdB = device->BeginCommandList_Independent(QUEUE_COMPUTE);
    ASSERT_TRUE(cmdB.IsValid());
    device->WaitCommandList(cmdB, cmdA);
    device->Barrier(&barrier, 1, cmdB);

    CommandList cmdC = device->BeginCommandList_Independent(QUEUE_COMPUTE);
    ASSERT_TRUE(cmdC.IsValid());
    device->WaitCommandList(cmdC, cmdB);
    device->Barrier(&barrier, 1, cmdC);

    CommandList cmdD = device->BeginCommandList_Independent(QUEUE_COMPUTE);
    ASSERT_TRUE(cmdD.IsValid());
    device->WaitCommandList(cmdD, cmdC);
    device->Barrier(&barrier, 1, cmdD);

    auto fenceA = device->SubmitCommandList_Independent(cmdA, "SubmitCommandListWithDependencies_A");
    auto fenceB = device->SubmitCommandList_Independent(cmdB, "SubmitCommandListWithDependencies_B");
    auto fenceC = device->SubmitCommandList_Independent(cmdC, "SubmitCommandListWithDependencies_C");
    auto fenceD = device->SubmitCommandList_Independent(cmdD, "SubmitCommandListWithDependencies_D");

    fenceD->CPUWait();

    EXPECT_TRUE(fenceA->IsSignaled());
    EXPECT_TRUE(fenceB->IsSignaled());
    EXPECT_TRUE(fenceC->IsSignaled());
    EXPECT_TRUE(fenceD->IsSignaled());
}