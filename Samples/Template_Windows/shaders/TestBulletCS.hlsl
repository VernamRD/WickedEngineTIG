struct BulletPosData
{
	float3 pos;
};

struct BulletVelData
{
	float3 vel;
};

cbuffer DeltaTimeCB : register(b0)
{
    float DeltaTime;
    float3 Padding;
};

RWStructuredBuffer<BulletPosData> posBuffer : register(u0); 
RWStructuredBuffer<BulletVelData> velBuffer : register(u1);

#define RootSig \
    "CBV(b0), " \
    "DescriptorTable(UAV(u0, numDescriptors=1), UAV(u1, numDescriptors=1))"
[RootSignature(RootSig)]

[numthreads(64, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
	uint index = DTid.x;

    BulletPosData newPosData = posBuffer[index];
    BulletVelData velData = velBuffer[index];
    
    newPosData.pos += velData.vel * DeltaTime;
    
    posBuffer[index] = newPosData;
}
