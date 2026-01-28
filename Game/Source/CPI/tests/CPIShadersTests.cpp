#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "CPI.h"
#include "CPIGraph.h"
#include <WickedEngine.h>
#include <wiGraphicsDevice_Vulkan.h>

#define GPU_CREATE_UAV_BUFFER(Device, Buffer, Type, Size)                                                         \
    wi::graphics::GPUBufferDesc desc##Buffer;                                                                     \
    desc##Buffer.usage = wi::graphics::Usage::DEFAULT;                                                            \
    desc##Buffer.bind_flags = wi::graphics::BindFlag::UNORDERED_ACCESS | wi::graphics::BindFlag::SHADER_RESOURCE; \
    desc##Buffer.misc_flags = wi::graphics::ResourceMiscFlag::BUFFER_STRUCTURED;                                  \
                                                                                                                  \
    desc##Buffer.stride = sizeof(Type);                                                                           \
    desc##Buffer.size = static_cast<uint64_t>(desc##Buffer.stride) * Size;                                        \
                                                                                                                  \
    Device->CreateBuffer(&desc##Buffer, nullptr, &Buffer);                                                        \
    Device->SetName(&Buffer, #Buffer);

#define GPU_CREATE_READBACK_BUFFER(Device, Buffer, Type, Size) \
    wi::graphics::GPUBufferDesc desc##Buffer;                  \
    desc##Buffer.usage = wi::graphics::Usage::READBACK;        \
    desc##Buffer.size = sizeof(Type) * Size;                   \
                                                               \
    Device->CreateBuffer(&desc##Buffer, nullptr, &Buffer);     \
    Device->SetName(&Buffer, #Buffer);

#define GPU_CREATE_CONST_BUFFER(Device, Buffer, Type)                  \
    wi::graphics::GPUBufferDesc desc##Buffer;                          \
    desc##Buffer.size = sizeof(Type);                                  \
    desc##Buffer.bind_flags = wi::graphics::BindFlag::CONSTANT_BUFFER; \
    desc##Buffer.usage = wi::graphics::Usage::DEFAULT;                 \
    Device->CreateBuffer(&desc##Buffer, nullptr, &Buffer);             \
    Device->SetName(&Buffer, #Buffer);

using namespace wi::graphics;

// Fixture for initializing GraphicsDevice once for all tests
class CPIShaderFixture : public ::testing::Test
{
protected:
    static std::unique_ptr<GraphicsDevice> device;
    static wi::platform::window_type window;

    static void SetUpTestSuite()
    {
        device = std::make_unique<GraphicsDevice_Vulkan>(nullptr, ValidationMode::Verbose, GPUPreference::Discrete);

        wi::graphics::GetDevice() = device.get();

        std::cout << "GraphicsDevice initialized for CPI tests." << std::endl;

        wi::renderer::SetShaderSourcePath(wi::helper::GetRootDir() + "shaders\\");
        std::cout << "ShaderSourcePath initialized for tests as " << wi::renderer::GetShaderSourcePath() << std::endl;
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

    void SetUp() override { ASSERT_NE(device, nullptr) << "GraphicsDevice does not initialized!"; }
};

std::unique_ptr<GraphicsDevice> CPIShaderFixture::device = nullptr;
wi::platform::window_type CPIShaderFixture::window = {};

struct BufferData
{
    int3 Data;
};

class CPITestShader : public Giperion::CPI::CPIShader
{
public:
    CPITestShader(int32_t elementCount = 1024, uint32_t threadGroupSize = 64)
        : CPIShader()
        , ELEMENT_COUNT(elementCount)
        , THREAD_GROUP_SIZE(threadGroupSize)
    {
    }
    void Initialize(wi::graphics::GraphicsDevice* device, wi::graphics::CommandList cmd) override
    {
        EXPECT_FALSE(m_initialized);
        m_initialized = true;

        EXPECT_TRUE(wi::renderer::LoadShader(wi::graphics::ShaderStage::CS, shader, "TestUAVCS.cso"));

        GPU_CREATE_UAV_BUFFER(device, dataBuffer, BufferData, ELEMENT_COUNT);

        std::vector<BufferData> initData(ELEMENT_COUNT);
        device->UpdateBuffer(&dataBuffer, initData.data(), cmd, ELEMENT_COUNT * sizeof(BufferData));

        wi::graphics::GPUBarrier dataBarrier = wi::graphics::GPUBarrier::Memory(&dataBuffer);
        device->Barrier(&dataBarrier, 1, cmd);

        std::cout << "Testing compute shader with " << ELEMENT_COUNT << " elements. " << dataBuffer.desc.size / 1024 << "KB total size buffer"
                  << std::endl;
    }

    void Execute(wi::graphics::GraphicsDevice* device, wi::graphics::CommandList cmd) override
    {
        EXPECT_TRUE(m_initialized);

        device->BindUAV(&dataBuffer, 0, cmd);
        device->BindComputeShader(&shader, cmd);
        uint32_t groupCount = (ELEMENT_COUNT + THREAD_GROUP_SIZE - 1) / THREAD_GROUP_SIZE;
        device->Dispatch(groupCount, 1, 1, cmd);
        auto barrier = wi::graphics::GPUBarrier::Memory(&dataBuffer);
        device->Barrier(&barrier, 1, cmd);
    }

    int32_t ELEMENT_COUNT = 1024;
    uint32_t THREAD_GROUP_SIZE = 128;

protected:
    wi::graphics::Shader shader;
    wi::graphics::GPUBuffer dataBuffer;
};

class CPIReadbackTestShader : public CPITestShader
{
public:
    CPIReadbackTestShader(int32_t elementCount = 1024, uint32_t threadGroupSize = 64)
        : CPITestShader(elementCount, threadGroupSize)
    {
    }

    void Initialize(wi::graphics::GraphicsDevice* device, wi::graphics::CommandList cmd) override
    {
        CPITestShader::Initialize(device, cmd);

        GPU_CREATE_READBACK_BUFFER(device, rbBuffer, BufferData, ELEMENT_COUNT);

        wi::graphics::GPUBarrier rbBarrier = wi::graphics::GPUBarrier::Memory(&rbBuffer);
        device->Barrier(&rbBarrier, 1, cmd);
    }

    void Readback(wi::graphics::GraphicsDevice* device, wi::graphics::CommandList cmd)
    {
        EXPECT_TRUE(m_initialized);

        device->CopyResource(&rbBuffer, &dataBuffer, cmd);

        wi::graphics::GPUBarrier barrier = wi::graphics::GPUBarrier::Memory(&rbBuffer);
        device->Barrier(&barrier, 1, cmd);
    }

    std::vector<BufferData> GetData()
    {
        EXPECT_TRUE(m_initialized);

        BufferData* mappedData = static_cast<BufferData*>(rbBuffer.mapped_data);
        EXPECT_THAT(mappedData, testing::NotNull());

        std::vector<BufferData> data(ELEMENT_COUNT);

        memcpy(data.data(), mappedData, rbBuffer.mapped_size);

        return data;
    }

private:
    wi::graphics::GPUBuffer rbBuffer;
};

TEST_F(CPIShaderFixture, ComputeShader)
{
    CPITestShader testShader;

    wi::graphics::CommandList initCmd = device->BeginCommandList_Independent(wi::graphics::QUEUE_COMPUTE, "ComputeShader_Init");
    wi::graphics::CommandList executeCmd = device->BeginCommandList_Independent(wi::graphics::QUEUE_COMPUTE, "ComputeShader_Execute");

    EXPECT_FALSE(testShader.IsInitialized());
    testShader.Initialize(device.get(), initCmd);
    testShader.Execute(device.get(), executeCmd);

    device->WaitCommandList(executeCmd, initCmd);

    device->SubmitCommandList_Independent(initCmd);
    auto fenceExecute = device->SubmitCommandList_Independent_Fenced(executeCmd);

    fenceExecute->CPUWait();

    EXPECT_TRUE(fenceExecute->IsSignaled());
}

TEST_F(CPIShaderFixture, ComputeShaderWithDependency)
{
    CPITestShader testShader1;
    CPITestShader testShader2;
    CPITestShader testShader3;

    wi::graphics::CommandList initCmd = device->BeginCommandList_Independent(wi::graphics::QUEUE_COMPUTE, "ComputeShader_Init");
    wi::graphics::CommandList executeCmd = device->BeginCommandList_Independent(wi::graphics::QUEUE_COMPUTE, "ComputeShader_Execute");

    EXPECT_FALSE(testShader1.IsInitialized());
    EXPECT_FALSE(testShader2.IsInitialized());
    EXPECT_FALSE(testShader3.IsInitialized());

    testShader1.Initialize(device.get(), initCmd);
    testShader2.Initialize(device.get(), initCmd);
    testShader3.Initialize(device.get(), initCmd);

    testShader1.Execute(device.get(), executeCmd);
    testShader2.Execute(device.get(), executeCmd);
    testShader3.Execute(device.get(), executeCmd);

    device->WaitCommandList(executeCmd, initCmd);

    device->SubmitCommandList_Independent(initCmd);
    auto fenceExecute = device->SubmitCommandList_Independent_Fenced(executeCmd);

    fenceExecute->CPUWait();

    EXPECT_TRUE(fenceExecute->IsSignaled());
}

TEST_F(CPIShaderFixture, ComputeShaderWithReadback)
{
    CPIReadbackTestShader testShader(1024, 64);

    wi::graphics::CommandList initCmd = device->BeginCommandList_Independent(wi::graphics::QUEUE_COMPUTE, "ComputeShader_Init");
    wi::graphics::CommandList executeCmd = device->BeginCommandList_Independent(wi::graphics::QUEUE_COMPUTE, "ComputeShader_Execute");
    wi::graphics::CommandList readBackCmd = device->BeginCommandList_Independent(wi::graphics::QUEUE_COPY, "ComputeShader_ReadBack");

    EXPECT_FALSE(testShader.IsInitialized());
    testShader.Initialize(device.get(), initCmd);
    testShader.Execute(device.get(), executeCmd);
    testShader.Readback(device.get(), readBackCmd);

    device->WaitCommandList(executeCmd, initCmd);
    device->WaitCommandList(readBackCmd, executeCmd);

    device->SubmitCommandList_Independent(initCmd);
    device->SubmitCommandList_Independent(executeCmd);
    auto fenceReadback = device->SubmitCommandList_Independent_Fenced(readBackCmd);

    fenceReadback->CPUWait();

    EXPECT_TRUE(fenceReadback->IsSignaled());

    auto data = testShader.GetData();
    for (int32_t i = 0; i < testShader.ELEMENT_COUNT; ++i)
    {
        int3 val1 = data[i].Data;
        EXPECT_EQ(val1, int3(1, 1, 1));
    }
}