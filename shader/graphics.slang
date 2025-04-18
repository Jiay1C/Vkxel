import util;

struct VertexInput
{
    float3 position;
    float3 normal;
    float3 color;
};

struct VertexOutput
{
    float4 localPosition;
    float4 worldPosition;
    float4 viewPosition;
    float4 clipPosition : SV_Position;
    float3 localNormal;
    float3 worldNormal;
    float3 viewNormal;
    float3 color;
};

struct FrameData{
    uint tick;
    float time;
};

struct SceneData{
    float4x4 viewMatrix;
    float4x4 projectionMatrix;
    float4 cameraPosition;
};

struct ConstantBufferPerFrame
{
    FrameData frame;
    SceneData scene;
};

struct ConstantBufferPerObject
{
    float4x4 transformMatrix;
};

[[vk::binding(0,0)]]
ConstantBuffer<ConstantBufferPerFrame> cbufferFrame;

[[vk::binding(0,1)]]
ConstantBuffer<ConstantBufferPerObject> cbufferObject;

float3x3 GetNormalMatrix(float4x4 m)
{
    return transpose(inverse((float3x3)m));
}

[shader("vertex")]
VertexOutput vertexMain(VertexInput input)
{
    VertexOutput output;
    float4x4 m = cbufferObject.transformMatrix;
    float4x4 v = cbufferFrame.scene.viewMatrix;
    float4x4 p = cbufferFrame.scene.projectionMatrix;

    float3x3 m_n = GetNormalMatrix(m);
    float3x3 v_n = GetNormalMatrix(v);

    // Position Transform
    output.localPosition = float4(input.position, 1.0);
    output.worldPosition = mul(m, output.localPosition);
    output.viewPosition = mul(v, output.worldPosition);
    output.clipPosition = mul(p, output.viewPosition);

    // Normal Transform
    output.localNormal = normalize(input.normal);
    output.worldNormal = normalize(mul(m_n, output.localNormal));
    output.viewNormal = normalize(mul(v_n, output.worldNormal));

    output.color = input.color;

    return output;
}

[shader("fragment")]
float4 fragmentMain(VertexOutput input) : SV_Target
{
    float time = cbufferFrame.frame.time;
    float3 lightPosition = {-10 * sin(time), 10, 10 * cos(time)};
    float3 lightColor = {1,1,1};

    float3 normal = normalize(input.worldNormal);
    float3 lightDir = normalize(lightPosition - input.worldPosition.xyz);
    float3 viewDir = normalize(cbufferFrame.scene.cameraPosition.xyz - input.worldPosition.xyz);

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
