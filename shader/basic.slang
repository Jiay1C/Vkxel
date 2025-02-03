import util;

struct VertexInput
{
    float3 position;
    float3 normal;
    float3 color;
};

struct VertexOutput
{
    float4 objectPosition;
    float4 worldPosition;
    float4 viewPosition;
    float4 clipPosition : SV_Position;
    float3 objectNormal;
    float3 worldNormal;
    float3 viewNormal;
    float3 color;
};

struct ConstantBufferPerFrame
{
    float4x4 modelMatrix;
    float4x4 viewMatrix;
    float4x4 projectionMatrix;
    float4 cameraPosition;
};

ConstantBuffer<ConstantBufferPerFrame> cbFrame;

float3x3 GetNormalMatrix(float4x4 m)
{
    return transpose(inverse((float3x3)m));
}

[shader("vertex")]
VertexOutput vertexMain(VertexInput input)
{
    VertexOutput output;
    float4x4 m = cbFrame.modelMatrix;
    float4x4 v = cbFrame.viewMatrix;
    float4x4 p = cbFrame.projectionMatrix;

    float3x3 m_n = GetNormalMatrix(m);
    float3x3 v_n = GetNormalMatrix(v);

    // Position Transform
    output.objectPosition = float4(input.position, 1.0);
    output.worldPosition = mul(m, output.objectPosition);
    output.viewPosition = mul(v, output.worldPosition);
    output.clipPosition = mul(p, output.viewPosition);

    // Normal Transform
    output.objectNormal = normalize(input.normal);
    output.worldNormal = normalize(mul(m_n, output.objectNormal));
    output.viewNormal = normalize(mul(v_n, output.worldNormal));

    output.color = input.color;

    return output;
}

[shader("fragment")]
float4 fragmentMain(VertexOutput input) : SV_Target
{

    float3 lightPosition = {-5,5,5};
    float3 lightColor = {1,1,1};

    float3 normal = normalize(input.worldNormal);
    float3 lightDir = normalize(lightPosition - input.worldPosition.xyz);
    float3 viewDir = normalize(cbFrame.cameraPosition.xyz - input.worldPosition.xyz);

    // Ambient Component
    float3 ambient = 0.1 * lightColor;

    // Diffuse Component
    float diff = max(dot(normal, lightDir), 0.0);
    float3 diffuse = diff * lightColor;

    // Specular Component (Blinn-Phong)
    float3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 16.0); // Shininess factor
    float3 specular = spec * lightColor;

    // Final Color
    float3 finalColor = (ambient + diffuse + specular) * input.color;
    return float4(finalColor, 1.0);
}
