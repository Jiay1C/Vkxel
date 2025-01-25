struct Vertex
{
    float3 position;
    float3 normal;
    float3 color;
}

struct VOut
{
    float4 position : SV_Position;
    float3 normal;
    float3 color;
}

[shader("vertex")]
VOut vertexMain(Vertex input)
{
    VOut output;
    output.position = float4(input.position, 1.0);
    output.normal = input.normal;
    output.color = input.color;
    return output;
}

[shader("fragment")]
float4 fragmentMain(VOut input) : SV_Target
{
    return float4(input.color, 1);
}