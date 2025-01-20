struct Vertex
{
    float3 position;
}

struct VOut
{
    float4 position : SV_Position;
}

[shader("vertex")]
VOut main(Vertex input)
{
    VOut output;
    output.position = float4(input.position, 1.0);
    return output;
}