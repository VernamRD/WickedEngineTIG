#include "TestDoubleUAV.h"

#include "TIGShaderUtils.h"
#include "wiRenderer.h"

void TestDoubleUAV::Initialize(wi::graphics::GraphicsDevice* Device)
{
	wi::renderer::LoadShader(wi::graphics::ShaderStage::CS, Shader, "TestDoubleUAV.cso");
	//
	// wi::graphics::GPUBufferDesc descPosBuffer;
	// descPosBuffer.usage = wi::graphics::Usage::DEFAULT;
	// descPosBuffer.bind_flags = wi::graphics::BindFlag::UNORDERED_ACCESS | wi::graphics::BindFlag::SHADER_RESOURCE;
	// descPosBuffer.misc_flags = wi::graphics::ResourceMiscFlag::BUFFER_STRUCTURED;
	//
	// descPosBuffer.stride = sizeof(DataUAV);
	// descPosBuffer.size = static_cast<uint64_t>(descPosBuffer.stride) * ELEMENT_COUNT;
	//
	// Device->CreateBuffer(&descPosBuffer, nullptr, &PosBuffer);
	// Device->SetName(&PosBuffer, "PosBuffer");

	GPU_CREATE_UAV_BUFFER(Device, PosBuffer, DataUAV, ELEMENT_COUNT);
	GPU_CREATE_UAV_BUFFER(Device, VelBuffer, DataUAV, ELEMENT_COUNT);

	// wi::graphics::GPUBufferDesc descVelBuffer;
	// descVelBuffer.usage = wi::graphics::Usage::DEFAULT;
	// descVelBuffer.bind_flags = wi::graphics::BindFlag::UNORDERED_ACCESS | wi::graphics::BindFlag::SHADER_RESOURCE;
	// descVelBuffer.misc_flags = wi::graphics::ResourceMiscFlag::BUFFER_STRUCTURED;
	//
	// descVelBuffer.stride = sizeof(DataUAV);
	// descVelBuffer.size = static_cast<uint64_t>(descVelBuffer.stride) * ELEMENT_COUNT;
	//
	// Device->CreateBuffer(&descVelBuffer, nullptr, &VelBuffer);
	// Device->SetName(&VelBuffer, "VelBuffer");

	InitCmd = Device->BeginCommandList(wi::graphics::QUEUE_COMPUTE);

	{
		std::vector<DataUAV> initPosData(ELEMENT_COUNT);
		Device->UpdateBuffer(&PosBuffer, initPosData.data(), InitCmd, ELEMENT_COUNT * sizeof(DataUAV));
		wi::graphics::GPUBarrier barrier = wi::graphics::GPUBarrier::Memory();
		Device->Barrier(&barrier, 1, InitCmd);
	}

	{
		std::vector<DataUAV> initVelData(ELEMENT_COUNT);
		for (auto& val : initVelData)
		{
			std::srand(1243);
			val.Data = float3(std::rand(), std::rand(), std::rand());
		}
		Device->UpdateBuffer(&VelBuffer, initVelData.data(), InitCmd, ELEMENT_COUNT * sizeof(DataUAV));
		wi::graphics::GPUBarrier barrier = wi::graphics::GPUBarrier::Memory();
		Device->Barrier(&barrier, 1, InitCmd);
	}

	Device->SubmitCommandLists();
}

void TestDoubleUAV::Execute(wi::graphics::GraphicsDevice* Device)
{
	ExecuteCmd = Device->BeginCommandList(wi::graphics::QUEUE_COMPUTE);

	Device->BindUAV(&PosBuffer, 0, ExecuteCmd);
	Device->BindUAV(&VelBuffer, 1, ExecuteCmd);
	Device->BindComputeShader(&Shader, ExecuteCmd);

	uint32_t groupCount = (ELEMENT_COUNT + THREAD_GROUP_SIZE - 1) / THREAD_GROUP_SIZE;
	Device->Dispatch(groupCount, 1, 1, ExecuteCmd);

	wi::graphics::GPUBarrier barrier = wi::graphics::GPUBarrier::Memory();
	Device->Barrier(&barrier, 1, ExecuteCmd);
}

std::unique_ptr<DataUAV[]> TestDoubleUAV::ReadBackResults(wi::graphics::GraphicsDevice* Device)
{
	wi::graphics::GPUBufferDesc descRB = {};
	descRB.usage = wi::graphics::Usage::READBACK;
	descRB.size = ELEMENT_COUNT * sizeof(DataUAV);

	wi::graphics::GPUBuffer RBBuffer;
	Device->CreateBuffer(&descRB, nullptr, &RBBuffer);
	Device->SetName(&RBBuffer, "ReadbackBuffer");

	auto cmd = Device->BeginCommandList(wi::graphics::QUEUE_COPY);
	Device->CopyResource(&RBBuffer, &PosBuffer, cmd);

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
