#pragma once

#define GPU_CREATE_UAV_BUFFER(Device, Buffer, Type, Size)																\
wi::graphics::GPUBufferDesc desc##Buffer;																		\
desc##Buffer.usage = wi::graphics::Usage::DEFAULT;																\
desc##Buffer.bind_flags = wi::graphics::BindFlag::UNORDERED_ACCESS | wi::graphics::BindFlag::SHADER_RESOURCE;	\
desc##Buffer.misc_flags = wi::graphics::ResourceMiscFlag::BUFFER_STRUCTURED;									\
																												\
desc##Buffer.stride = sizeof(Type);																				\
desc##Buffer.size = static_cast<uint64_t>(desc##Buffer.stride) * Size;											\
																												\
Device->CreateBuffer(&desc##Buffer, nullptr, &Buffer);															\
Device->SetName(&Buffer, #Buffer);

#define GPU_CREATE_CONST_BUFFER(Device, Buffer, Type)																	\
wi::graphics::GPUBufferDesc desc##Buffer;																			\
desc##Buffer.size = sizeof(Type);																				\
desc##Buffer.bind_flags = wi::graphics::BindFlag::CONSTANT_BUFFER;												\
desc##Buffer.usage = wi::graphics::Usage::DEFAULT;																\
Device->CreateBuffer(&desc##Buffer, nullptr, &Buffer);														\
Device->SetName(&Buffer, #Buffer);

namespace TIG::graphics
{

}
