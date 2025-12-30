#pragma once

#include "wiGraphicsDevice.h"
#include "shaders/ShaderInterop.h"

struct BulletPosType
{
	float3 Data;
};

struct BulletVelType
{
	float3 Vel;
};

struct BulletCBType
{
	float DeltaTime;
	float3 Padding;
};

class TestBulletCS
{
public:
	void Initialize(wi::graphics::GraphicsDevice* Device);
	void Execute(wi::graphics::GraphicsDevice* Device, float DeltaTime);
	std::unique_ptr<BulletPosType[]> ReadBackResults(wi::graphics::GraphicsDevice* Device);

private:
	const int32_t MAX_ELEMENT_COUNT = 10000000;
	const uint32_t THREAD_GROUP_SIZE = 64;

	wi::graphics::Shader Shader;
	wi::graphics::CommandList InitCmd;
	wi::graphics::CommandList ExecuteCmd;
	wi::graphics::CommandList ReadbackCMD;

	wi::graphics::GPUBuffer PosBuffer;
	wi::graphics::GPUBuffer VelBuffer;
	wi::graphics::GPUBuffer CBBuffer;
};
