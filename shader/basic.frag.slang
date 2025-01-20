struct VOut
{
    float4 position : SV_Position;
}

[shader("fragment")]
float4 main(VOut input) : SV_Target
{
    return float4(1,1,1,1);
}