struct Input
{
    uint3 thread;
    float3 min_bound;
    float3 max_bound;
};

StructuredBuffer<Input> input;
RWStructuredBuffer<float> output;

float sdf_sphere(float3 position)
{
    return length(position) - 1;
}

[shader("compute")]
[numthreads(4,4,4)]
void computeMain(uint3 threadId : SV_DispatchThreadID)
{
    Input data = input[0];
    float3 position = float3(threadId) / data.thread * (data.max_bound - data.min_bound) + data.min_bound;
    output[threadId.x * data.thread.y * data.thread.z + threadId.y * data.thread.z + threadId.z] = sdf_sphere(position);
}