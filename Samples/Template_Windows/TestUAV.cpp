#include "TestUAV.h"

#include "wiRenderer.h"

void TestUAV::Initialize(wi::graphics::GraphicsDevice* Device)
{
	wi::renderer::LoadShader(wi::graphics::ShaderStage::CS, Shader, "TestUAVCS.cso");

	wi::graphics::GPUBufferDesc descDataBuffer;
	descDataBuffer.usage = wi::graphics::Usage::DEFAULT;
	descDataBuffer.bind_flags = wi::graphics::BindFlag::UNORDERED_ACCESS | wi::graphics::BindFlag::SHADER_RESOURCE;
	descDataBuffer.misc_flags = wi::graphics::ResourceMiscFlag::BUFFER_STRUCTURED;

	descDataBuffer.stride = sizeof(float3);
	descDataBuffer.size = static_cast<uint64_t>(descDataBuffer.stride) * ELEMENT_COUNT;

	Device->CreateBuffer(&descDataBuffer, nullptr, &DataBuffer);
	Device->SetName(&DataBuffer, "TestUAV");

	InitCmd = Device->BeginCommandList(wi::graphics::QUEUE_COMPUTE);

	std::vector<DataUAV> initData(ELEMENT_COUNT);
	Device->UpdateBuffer(&DataBuffer, initData.data(), InitCmd, ELEMENT_COUNT * sizeof(DataUAV));
	wi::graphics::GPUBarrier barrier = wi::graphics::GPUBarrier::Memory();
	Device->Barrier(&barrier, 1, InitCmd);

	Device->SubmitCommandLists();
}

void TestUAV::Execute(wi::graphics::GraphicsDevice* Device)
{
	ExecuteCmd = Device->BeginCommandList(wi::graphics::QUEUE_COMPUTE);

	Device->BindUAV(&DataBuffer, 0, ExecuteCmd);
	Device->BindComputeShader(&Shader, ExecuteCmd);

	uint32_t groupCount = (ELEMENT_COUNT + THREAD_GROUP_SIZE - 1) / THREAD_GROUP_SIZE;
	Device->Dispatch(groupCount, 1, 1, ExecuteCmd);

	wi::graphics::GPUBarrier barrier = wi::graphics::GPUBarrier::Memory();
	Device->Barrier(&barrier, 1, ExecuteCmd);
}

std::unique_ptr<DataUAV[]> TestUAV::ReadBackResults(wi::graphics::GraphicsDevice* Device)
{
	wi::graphics::GPUBufferDesc descRB = {};
	descRB.usage = wi::graphics::Usage::READBACK;
	descRB.size = ELEMENT_COUNT * sizeof(DataUAV);

	wi::graphics::GPUBuffer RBBuffer;
	Device->CreateBuffer(&descRB, nullptr, &RBBuffer);
	Device->SetName(&RBBuffer, "ReadbackBuffer");

	auto cmd = Device->BeginCommandList(wi::graphics::QUEUE_COPY);
	Device->CopyResource(&RBBuffer, &DataBuffer, cmd);

	wi::graphics::GPUBarrier barrier = wi::graphics::GPUBarrier::Memory();
	Device->Barrier(&barrier, 1, cmd);

	Device->SubmitCommandLists();
	Device->WaitForGPU();

	DataUAV* mappedData = static_cast<DataUAV*>(RBBuffer.mapped_data);
	if (!mappedData) return nullptr;

	auto dataPtr = std::make_unique<DataUAV[]>(ELEMENT_COUNT);

	memcpy(dataPtr.get(), mappedData, descRB.size);

	return std::move(dataPtr);
}
