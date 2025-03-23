StructuredBuffer<float> input_buffer;
RWStructuredBuffer<float> output_buffer;

[shader("compute")]
[numthreads(1,1,1)]
void computeMain(uint3 threadId : SV_DispatchThreadID)
{
    uint index = threadId.x;
    output_buffer[index] = input_buffer[index] + 1;
}