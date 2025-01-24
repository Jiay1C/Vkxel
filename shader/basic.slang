struct Vertex
{
    float3 position;
}

struct VOut
{
    float4 position : SV_Position;
}

[shader("vertex")]
VOut vertexMain(Vertex input)
{
    VOut output;
    output.position = float4(input.position, 1.0);
    return output;
}

[shader("fragment")]
float4 fragmentMain(VOut input) : SV_Target
{
    return float4(1,1,1,1);
}