#pragma once
#include "wiGraphics.h"
#include "wiGraphicsDevice.h"
#include "shaders/ShaderInterop.h"

struct DataUAV
{
	float3 Data;
};

class TestUAV
{
public:
	void Initialize(wi::graphics::GraphicsDevice* Device);
	void Execute(wi::graphics::GraphicsDevice* Device);
	std::unique_ptr<DataUAV[]> ReadBackResults(wi::graphics::GraphicsDevice* Device);

private:
	const int32_t ELEMENT_COUNT = 64;
	const uint32_t THREAD_GROUP_SIZE = 64;

	wi::graphics::Shader Shader;
	wi::graphics::CommandList InitCmd;
	wi::graphics::CommandList ExecuteCmd;

	wi::graphics::GPUBuffer DataBuffer;
};
