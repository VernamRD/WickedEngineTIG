#include "TestBulletCS.h"

#include <random>

#include "TIGShaderUtils.h"
#include "wiProfiler.h"
#include "wiRenderer.h"

void TestBulletCS::Initialize(wi::graphics::GraphicsDevice* Device)
{
	wi::renderer::LoadShader(wi::graphics::ShaderStage::CS, Shader, "TestBulletCS.cso");

	GPU_CREATE_CONST_BUFFER(Device, CBBuffer, BulletCBType)
	GPU_CREATE_UAV_BUFFER(Device, PosBuffer, BulletPosType, MAX_ELEMENT_COUNT)
	GPU_CREATE_UAV_BUFFER(Device, VelBuffer, BulletVelType, MAX_ELEMENT_COUNT)

	InitCmd = Device->BeginCommandList(wi::graphics::QUEUE_COMPUTE);

	{
		std::vector<BulletPosType> initPosData(MAX_ELEMENT_COUNT);
		Device->UpdateBuffer(&PosBuffer, initPosData.data(), InitCmd, MAX_ELEMENT_COUNT * sizeof(BulletPosType));
	}

	{
		std::srand(std::_Random_device());

		std::vector<BulletVelType> initVelData(MAX_ELEMENT_COUNT);
		for (auto& val : initVelData)
		{
			val.Vel = float3(std::rand(), std::rand(), std::rand());
		}
		Device->UpdateBuffer(&VelBuffer, initVelData.data(), InitCmd, MAX_ELEMENT_COUNT * sizeof(BulletVelType));
		wi::graphics::GPUBarrier barrier = wi::graphics::GPUBarrier::Memory();
		Device->Barrier(&barrier, 1, InitCmd);
	}

	Device->SubmitCommandLists();
}

void TestBulletCS::Execute(wi::graphics::GraphicsDevice* Device, float DeltaTime)
{
	ExecuteCmd = Device->BeginCommandList(wi::graphics::QUEUE_COMPUTE);

	Device->EventBegin("StartBulletMovement", ExecuteCmd);
	auto Range = wi::profiler::BeginRangeGPU("BulletMovement", ExecuteCmd);

	BulletCBType CBData;
	CBData.DeltaTime = DeltaTime;
	Device->UpdateBuffer(&CBBuffer, &CBData, ExecuteCmd, sizeof(BulletCBType));

	Device->BindUAV(&PosBuffer, 0, ExecuteCmd);
	Device->BindUAV(&VelBuffer, 1, ExecuteCmd);
	Device->BindComputeShader(&Shader, ExecuteCmd);

	Device->BindConstantBuffer(&CBBuffer, 0, ExecuteCmd);

	uint32_t groupCount = (MAX_ELEMENT_COUNT + THREAD_GROUP_SIZE - 1) / THREAD_GROUP_SIZE;
	Device->Dispatch(groupCount, 1, 1, ExecuteCmd);

	wi::graphics::GPUBarrier barrier = wi::graphics::GPUBarrier::Memory();
	Device->Barrier(&barrier, 1, ExecuteCmd);

	wi::profiler::EndRange(Range);
	Device->EventEnd(ExecuteCmd);

	auto CPUScope = wi::profiler::BeginRangeCPU("BulletsUpdate_Execute");

	Device->SubmitCommandLists();

	wi::profiler::EndRange(CPUScope);
}

std::unique_ptr<BulletPosType[]> TestBulletCS::ReadBackResults(wi::graphics::GraphicsDevice* Device)
{
	ReadbackCMD = Device->BeginCommandList(wi::graphics::QUEUE_COMPUTE);

	Device->EventBegin("StartBulletMovement_RB", ReadbackCMD);
	auto Range = wi::profiler::BeginRangeGPU("BulletMovement_RB", ReadbackCMD);

	wi::graphics::GPUBufferDesc descRB = {};
	descRB.usage = wi::graphics::Usage::READBACK;
	descRB.size = MAX_ELEMENT_COUNT * sizeof(BulletPosType);

	wi::graphics::GPUBuffer RBBuffer;
	Device->CreateBuffer(&descRB, nullptr, &RBBuffer);
	Device->SetName(&RBBuffer, "ReadbackBuffer");

	Device->CopyResource(&RBBuffer, &PosBuffer, ReadbackCMD);

	wi::graphics::GPUBarrier barrier = wi::graphics::GPUBarrier::Memory();
	Device->Barrier(&barrier, 1, ReadbackCMD);

	wi::profiler::EndRange(Range);
	Device->EventEnd(ReadbackCMD);

	{
		auto scope = wi::profiler::BeginRangeCPU("BulletsUpdate_WaitGPU");
		Device->SubmitCommandLists();
		Device->WaitForGPU();
		wi::profiler::EndRange(scope);
	}

	auto scope = wi::profiler::BeginRangeCPU("BulletsUpdate_Copy");

	BulletPosType* mappedData = static_cast<BulletPosType*>(RBBuffer.mapped_data);
	if (!mappedData) return nullptr;

	auto dataPtr = std::make_unique<BulletPosType[]>(MAX_ELEMENT_COUNT);

	memcpy(dataPtr.get(), mappedData, descRB.size);

	wi::profiler::EndRange(scope);

	return std::move(dataPtr);
}
