struct VertexInput
{
    float3 position;
    float3 normal;
    float3 color;
}

struct VertexOutput
{
    float4 position : SV_Position;
    float3 normal;
    float3 color;
}

struct ConstantBufferPerFrame
{
    float4x4 modelMatrix;
    float4x4 viewMatrix;
    float4x4 projectionMatrix;
}

ConstantBuffer<ConstantBufferPerFrame> cbFrame;

[shader("vertex")]
VertexOutput vertexMain(VertexInput input)
{
    VertexOutput output;
    float4x4 m = cbFrame.modelMatrix;
    float4x4 v = cbFrame.viewMatrix;
    float4x4 p = cbFrame.projectionMatrix;
    float4 position = float4(input.position, 1.0);
    position = mul(m, position);
    position = mul(v, position);
    position = mul(p, position);
    output.position = position;
    output.normal = input.normal;
    output.color = input.color;
    return output;
}

[shader("fragment")]
float4 fragmentMain(VertexOutput input) : SV_Target
{
    return float4(input.color, 1);
}