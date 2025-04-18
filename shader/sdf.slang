// Primitive

enum SurfaceType : uint {
    None = 0,
    Primitive = 1,
    Custom = 2,
    CSG = 3,
};
enum PrimitiveType : uint {
    None = 0,
    Sphere = 1,
    Box = 2,
    Capsule = 3,
};
enum CSGType : uint {
    None = 0,
    Unionize = 1,
    Intersect = 2,
    Subtract = 3,
};
struct SDFSurfaceGPU {
    float4x4 gridToLocal;
    SurfaceType surfaceType;
    PrimitiveType primitiveType;
    CSGType csgType;
    float csgSmoothFactor;
    uint numChildren;
    // ----------- 84 bytes ----------- //
    uint padding0;
    uint padding1;
    uint padding2;
    // ---------- 96 bytes ----------- //
};
static const uint MAX_SDF_SURFACE_GPU = 32;

float sdfSphere(float3 p) {
    return length(p) - 1;
}

float sdfBox(float3 p) {
    float3 q = abs(p) - float3(1.0f, 1.0f, 1.0f);
    return length(max(q, 0.0f)) + min(max(q.x, max(q.y, q.z)), 0.0f);
};

float sdfCapsule(float3 p) {
    float3 q = p;
    q.y -= clamp(q.y, -0.5f, 0.5f);
    return length(q) - 0.5f;
};

// CSG Operation

float csgUnion(float d1, float d2)
{
    return min(d1, d2);
}

float csgSubtraction(float d1, float d2)
{
    return max(d1, -d2);
}

float csgIntersection(float d1, float d2)
{
    return max(d1, d2);
}

float csgSmoothUnion(float d1, float d2, float k)
{
    float h = clamp(0.5 + 0.5 * (d2 - d1) / k, 0.0, 1.0);
    return lerp(d2, d1, h) - k * h * (1.0-h);
}

float csgSmoothSubtraction(float d1, float d2, float k)
{
    float h = clamp(0.5 - 0.5 * (d2 + d1) / k, 0.0, 1.0);
    return lerp(d1, -d2, h) + k * h * (1.0 - h);
}

float csgSmoothIntersection(float d1, float d2, float k)
{
    float h = clamp(0.5 - 0.5 * (d2 - d1) / k, 0.0, 1.0);
    return lerp(d2, d1, h) + k * h * (1.0 - h);
}
