struct BulletPosData
{
	float3 pos;
};
struct BulletVelData
{
	float3 vel;
};

cbuffer DeltaTimeCB : register(b0){

    float DeltaTime;
    float3 Padding;
}

RWStructuredBuffer<BulletPosData> posBuffer : register(u0); 
RWStructuredBuffer<BulletVelData> velBuffer : register(u1);

[numthreads(64, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
	uint index = DTid.x;

    float3 pos = posBuffer[index].pos;
    float3 vel = velBuffer[index].vel;
    
    pos += 1 * DeltaTime;
    
    posBuffer[index].pos = pos;
}
