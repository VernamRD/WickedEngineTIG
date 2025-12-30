#include "TestCompute.h"

#include "wiProfiler.h"
#include "wiRenderer.h"

struct DeltaTimeCB
{
	float DeltaTime;
	float3 Padding;
};

void TestCompute::initialize(wi::graphics::GraphicsDevice* Device, wi::graphics::CommandList CommandList)
{
	wi::renderer::LoadShader(wi::graphics::ShaderStage::CS, TestShader, "TestBulletCS.cso");

	{
		wi::graphics::GPUBufferDesc descPosBuffer;
		descPosBuffer.usage = wi::graphics::Usage::DEFAULT;
		descPosBuffer.bind_flags = wi::graphics::BindFlag::UNORDERED_ACCESS | wi::graphics::BindFlag::SHADER_RESOURCE;
		descPosBuffer.misc_flags = wi::graphics::ResourceMiscFlag::BUFFER_STRUCTURED;

		descPosBuffer.stride = sizeof(BulletLocation);
		descPosBuffer.size = static_cast<uint64_t>(descPosBuffer.stride) * ELEMENTS_COUNT;

		Device->CreateBuffer(&descPosBuffer, nullptr, &posBuffer);
		Device->SetName(&posBuffer, "BulletPositionBuffer");

		std::vector<BulletLocation> initialPosData(ELEMENTS_COUNT);
		Device->UpdateBuffer(&posBuffer, initialPosData.data(), CommandList, ELEMENTS_COUNT * sizeof(BulletLocation));
	}

	{
		wi::graphics::GPUBufferDesc descVelBuffer;
		descVelBuffer.usage = wi::graphics::Usage::DEFAULT;
		descVelBuffer.bind_flags = wi::graphics::BindFlag::UNORDERED_ACCESS | wi::graphics::BindFlag::SHADER_RESOURCE;
		descVelBuffer.misc_flags = wi::graphics::ResourceMiscFlag::BUFFER_STRUCTURED;

		descVelBuffer.stride = sizeof(BulletVelocity);
		descVelBuffer.size = static_cast<uint64_t>(descVelBuffer.stride) * ELEMENTS_COUNT;

		Device->CreateBuffer(&descVelBuffer, nullptr, &velBuffer);
		Device->SetName(&velBuffer, "BulletVelocityBuffer");

		std::vector<BulletVelocity> initialVelData(ELEMENTS_COUNT);
		for (int32_t i = 0; i < ELEMENTS_COUNT; ++i)
		{
			initialVelData[i].velocity = float3(10, 10, 10);
		}
		Device->UpdateBuffer(&velBuffer, initialVelData.data(), CommandList,ELEMENTS_COUNT * sizeof(BulletVelocity));
	}

	wi::graphics::GPUBufferDesc cbDesc = {};
	cbDesc.size = sizeof(DeltaTimeCB);
	cbDesc.bind_flags = wi::graphics::BindFlag::CONSTANT_BUFFER;
	cbDesc.usage = wi::graphics::Usage::DEFAULT;

	Device->CreateBuffer(&cbDesc, nullptr, &constantBuffer);
	Device->SetName(&constantBuffer, "BulletConstantBuffer");
}

void TestCompute::Execute(const float DeltaTime, wi::graphics::GraphicsDevice* Device, wi::graphics::CommandList CommandList)
{
	Device->EventBegin("Bullet Update", CommandList);

	Device->BindUAV(&posBuffer, 0, CommandList);
	Device->BindUAV(&velBuffer, 1, CommandList);

	Device->BindComputeShader(&TestShader, CommandList);

	DeltaTimeCB cb = {};
	cb.DeltaTime = DeltaTime;
	Device->UpdateBuffer(&constantBuffer, &cb, CommandList, sizeof(DeltaTimeCB));
	Device->BindConstantBuffer(&constantBuffer, 0, CommandList);

	uint32_t groupCount = (ELEMENTS_COUNT + THREAD_GROUP_SIZE - 1) / THREAD_GROUP_SIZE;
	Device->Dispatch(groupCount, 1, 1, CommandList);

	auto barrier = wi::graphics::GPUBarrier::Memory();
	Device->Barrier(&barrier, 1, CommandList);

	Device->EventEnd(CommandList);
}

std::unique_ptr<BulletLocation[]> TestCompute::ReadBackResults(wi::graphics::GraphicsDevice* Device, wi::graphics::CommandList CommandList)
{
	wi::graphics::GPUBufferDesc stagingDesc = {};
	stagingDesc.usage = wi::graphics::Usage::READBACK;
	stagingDesc.size = sizeof(BulletLocation) * ELEMENTS_COUNT;

	wi::graphics::GPUBuffer stagingBuffer;
	Device->CreateBuffer(&stagingDesc, nullptr, &stagingBuffer);
	Device->SetName(&stagingBuffer, "BulletBuffer_Readback");

	Device->CopyResource(&stagingBuffer, &posBuffer, CommandList);

	wi::graphics::GPUBarrier barrier = wi::graphics::GPUBarrier::Memory();
	Device->Barrier(&barrier, 1, CommandList);

	Device->SubmitCommandLists();
	Device->WaitForGPU();

	BulletLocation* mappedData = static_cast<BulletLocation*>(stagingBuffer.mapped_data);
	if (!mappedData) return nullptr;

	auto dataPtr = std::make_unique<BulletLocation[]>(ELEMENTS_COUNT);

	memcpy(dataPtr.get(), mappedData, stagingDesc.size);

	return std::move(dataPtr);
}
