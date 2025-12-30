struct TestData
{
	float3 pos;
};

RWStructuredBuffer<TestData> Data : register(u0);

cbuffer DeltaTimeCB : register(b0)
{
    float DeltaTime;
    float3 Padding;
}

#define RootSig \
    "CBV(b0), " \
    "DescriptorTable(UAV(u0, numDescriptors=1))"
[RootSignature(RootSig)]

[numthreads(64, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    TestData newPos = Data[DTid.x];
    newPos.pos += float3(1, 1, 1) * DeltaTime;
    Data[DTid.x] = newPos;
}