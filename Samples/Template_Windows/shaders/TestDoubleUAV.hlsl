struct TestData
{
	float3 val;
};

RWStructuredBuffer<TestData> Pos : register(u0);
RWStructuredBuffer<TestData> Vel : register(u1);

#define RootSig \
    "DescriptorTable(UAV(u0, numDescriptors=1), UAV(u1, numDescriptors=1))"
[RootSignature(RootSig)]

[numthreads(64, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    TestData newPos = Pos[DTid.x];
    TestData currentVel = Vel[DTid.x];
    newPos.val += currentVel.val;
    Pos[DTid.x] = newPos;
}