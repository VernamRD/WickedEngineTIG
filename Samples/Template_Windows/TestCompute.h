#pragma once

#include "wiGraphics.h"
#include "wiGraphicsDevice.h"
#include "shaders/ShaderInterop.h"

namespace wi::graphics
{
	class GraphicsDevice;
}

struct BulletLocation
{
	float3 position;
};

struct BulletVelocity
{
	float3 velocity;
};

class TestCompute
{
private:

	wi::graphics::Shader TestShader;
	wi::graphics::GPUBuffer posBuffer;
	wi::graphics::GPUBuffer velBuffer;
	wi::graphics::GPUBuffer constantBuffer;

public:
	const uint32_t ELEMENTS_COUNT = 500000;
	const uint32_t THREAD_GROUP_SIZE = 64;

	void initialize(wi::graphics::GraphicsDevice* Device, wi::graphics::CommandList CommandList);
	void Execute(const float DeltaTime, wi::graphics::GraphicsDevice* Device, wi::graphics::CommandList CommandList);
	std::unique_ptr<BulletLocation[]> ReadBackResults(wi::graphics::GraphicsDevice* Device, wi::graphics::CommandList CommandList);
};
