float sdf(float3 position) {
    return sdfCapsule(position);
}

float sdfSphere(float3 p) {
    return length(p) - 1;
}

float sdfBox(float3 p) {
    float3 q = abs(p) - float3(0.5f, 0.5f, 0.5f);
    return length(max(q, 0.0f)) + min(max(q.x, max(q.y, q.z)), 0.0f);
};

float sdfCapsule(float3 p) {
    float3 q = p;
    q.y -= clamp(q.y, -0.5f, 0.5f);
    return length(q) - 0.5f;
};