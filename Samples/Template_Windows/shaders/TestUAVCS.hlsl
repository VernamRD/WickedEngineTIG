struct TestData
{
	float3 pos;
};

RWStructuredBuffer<TestData> Data : register(u0);

#define RootSig \
    "DescriptorTable(UAV(u0, numDescriptors=1))"
[RootSignature(RootSig)]

[numthreads(64, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    TestData newPos = Data[DTid.x];
    newPos.pos += float3(1, 1, 1);
    Data[DTid.x] = newPos;
}