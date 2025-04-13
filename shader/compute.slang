import sdf;

struct DualContouringArguments {
    uint3 gridSize;
    float3 minBound;
    float3 maxBound;
    float normalDelta;
    uint schmitzIterationCount;
    float schmitzStepSize;
    float time;
};

struct DualContouringResults {
    uint vertexCount;
    uint indexCount;
};

struct VertexData {
    float3 position;
    float3 normal;
    float3 color;
}

struct OffsetData {
    int3 pointOffset;
    int3 cellOffset[4];
};

[[vk::binding(0)]] StructuredBuffer<DualContouringArguments> args;
[[vk::binding(1)]] RWStructuredBuffer<DualContouringResults> results;
[[vk::binding(2)]] RWStructuredBuffer<float4> grid;
[[vk::binding(3)]] RWStructuredBuffer<uint> grid_vertex_index;
[[vk::binding(4)]] RWStructuredBuffer<VertexData> vertices;
[[vk::binding(5)]] RWStructuredBuffer<uint> indices;

static const int3 VOXEL_POINT[8] = {
    int3(0, 0, 0), int3(1, 0, 0), int3(1, 0, 1), int3(0, 0, 1),
    int3(0, 1, 0), int3(1, 1, 0), int3(1, 1, 1), int3(0, 1, 1)
};

static const int2 VOXEL_EDGE[12] = {
    int2(0, 1), int2(1, 2), int2(2, 3), int2(3, 0),
    int2(4, 5), int2(5, 6), int2(6, 7), int2(7, 4),
    int2(0, 4), int2(1, 5), int2(2, 6), int2(3, 7)
};

static const OffsetData POINT_OFFSET[3] = {
    { int3(1, 0, 0), { int3(0, -1, -1), int3(0, 0, -1), int3(0, -1, 0), int3(0, 0, 0) } },
    { int3(0, 1, 0), { int3(-1, 0, -1), int3(-1, 0, 0), int3(0, 0, -1), int3(0, 0, 0) } },
    { int3(0, 0, 1), { int3(-1, -1, 0), int3(0, -1, 0), int3(-1, 0, 0), int3(0, 0, 0) } }
};

static const uint TRIANGLE_INDEX_FRONT[6] = { 0, 2, 1, 1, 2, 3 };
static const uint TRIANGLE_INDEX_BACK[6] = { 0, 1, 2, 1, 3, 2 };

uint getIndex1D(uint3 size, uint3 index) {
    return dot(index, uint3(size.y * size.z, size.z, 1));
}

uint getIndex1D(uint3 index) {
    return getIndex1D(args[0].gridSize, index);
}

float3 grid2World(float3 cellIndex) {
    return cellIndex * (args[0].maxBound - args[0].minBound) / args[0].gridSize + args[0].minBound;
}

float sdf(float3 position) {
    const float dist = 0.6;
    float3 offset = float3(sin(args[0].time) * dist + dist, 0, 0);
    return csgSmoothUnion(sdfSphere(position - offset), sdfSphere(position + offset), 0.5);
}

float3 calcNormal(float3 position) {
    float normal_delta = args[0].normalDelta;
    float3 x_delta = float3(normal_delta, 0, 0);
    float3 y_delta = float3(0, normal_delta, 0);
    float3 z_delta = float3(0, 0, normal_delta);

    return normalize(float3(
        (sdf(position + x_delta) - sdf(position - x_delta)) * 0.5f / normal_delta,
        (sdf(position + y_delta) - sdf(position - y_delta)) * 0.5f / normal_delta,
        (sdf(position + z_delta) - sdf(position - z_delta)) * 0.5f / normal_delta
    ));
}

[shader("compute")]
[numthreads(4, 4, 4)]
void DualContouringStep0(uint3 threadId : SV_DispatchThreadID) {
    DualContouringArguments arg = args[0];

    // First Step: Generate SDF Grid
    if (all(threadId < arg.gridSize))
    {
        uint3 point_id = threadId;
        float3 position = grid2World(point_id);
        float4 point_data = float4(calcNormal(position), sdf(position));

        grid[getIndex1D(threadId)] = point_data;
    }
}

[shader("compute")]
[numthreads(4, 4, 4)]
void DualContouringStep1(uint3 threadId: SV_DispatchThreadID) {
    DualContouringArguments arg = args[0];

    // Second Step: Generate Vertices
    if (all(threadId < (arg.gridSize - uint3(1)))) {
        uint3 cell_id = threadId;
        VertexData[12] intersections;
        uint intersection_count = 0;

        [unroll]
        for (uint i = 0; i < 12; ++i) {
            int2 edge = VOXEL_EDGE[i];
            int3 p0_local = VOXEL_POINT[edge.x];
            int3 p1_local = VOXEL_POINT[edge.y];

            int3 p0 = cell_id + p0_local;
            int3 p1 = cell_id + p1_local;

            float4 p0_data = grid[getIndex1D(p0)];
            float4 p1_data = grid[getIndex1D(p1)];

            if ((p0_data.w <= 0 && p1_data.w >= 0) || (p0_data.w >= 0 && p1_data.w <= 0)) {
                float interpolate_factor = clamp(abs(p0_data.w) / (abs(p0_data.w) + abs(p1_data.w)), 0.0, 1.0);
                float3 position_local = lerp(float3(p0_local), float3(p1_local), interpolate_factor);
                float3 normal_world = normalize(lerp(p0_data.xyz, p1_data.xyz, interpolate_factor));
                intersections[intersection_count++] = {position_local, normal_world, float3(1,1,1)};
            }
        }

        if (intersection_count > 0) {
            VertexData center = {float3(0), float3(0), float3(0)};

            [unroll(12)]
            for (uint i = 0; i < intersection_count; ++i) {
                center.position += intersections[i].position;
                center.normal += intersections[i].normal;
                center.color += intersections[i].color;
            }
            center.position /= intersection_count;
            center.normal = normalize(center.normal);
            center.color /= intersection_count;

            float3[8] force;

            [unroll]
            for (uint index = 0; index < 8; ++index) {
                force[index] = 0;

                [unroll(12)]
                for (uint i = 0; i < intersection_count; ++i) {
                    float distance = dot(intersections[i].normal, float3(VOXEL_POINT[index]) - intersections[i].position);
                    float3 corner2plane = -distance * intersections[i].normal;
                    force[index] += corner2plane;
                }
            }

            for (uint count = 0; count < arg.schmitzIterationCount; ++count) {
                float3 force00 = lerp(force[0], force[1], center.position.x);
                float3 force01 = lerp(force[3], force[2], center.position.x);
                float3 force02 = lerp(force[4], force[5], center.position.x);
                float3 force03 = lerp(force[7], force[6], center.position.x);

                float3 force10 = lerp(force00, force02, center.position.y);
                float3 force11 = lerp(force01, force03, center.position.y);

                float3 force20 = lerp(force10, force11, center.position.z);

                center.position += force20 * arg.schmitzStepSize;
            }

            center.position = grid2World(float3(cell_id) + center.position);

            uint global_vertex_index;
            InterlockedAdd(results[0].vertexCount, 1, global_vertex_index);

            grid_vertex_index[getIndex1D(cell_id)] = global_vertex_index;
            vertices[global_vertex_index] = center;
        }
        else {
            grid_vertex_index[getIndex1D(cell_id)] = ~0;
        }
    }
}

[shader("compute")]
[numthreads(4, 4, 4)]
void DualContouringStep2(uint3 threadId: SV_DispatchThreadID) {
    DualContouringArguments arg = args[0];

    // Third Step: Generate Indices
    if (all(threadId >= uint3(1)) && all(threadId < (arg.gridSize - uint3(1)))) {
        [unroll]
        for (uint i = 0; i < 3; ++i) {
            OffsetData offset = POINT_OFFSET[i];
            uint3 p0 = threadId;
            uint3 p1 = p0 + offset.pointOffset;

            float p0_value = grid[getIndex1D(p0)].w;
            float p1_value = grid[getIndex1D(p1)].w;

            if ((p0_value <= 0 && p1_value >= 0) || (p0_value >= 0 && p1_value <= 0)) {
                uint[4] vertex_index;

                [unroll]
                for (uint index = 0; index < 4; ++index) {
                    uint3 cell_id = p0 + offset.cellOffset[index];
                    vertex_index[index] = grid_vertex_index[getIndex1D(cell_id)];
                }

                uint triangle_index[6] = (p0_value >= 0 && p1_value <= 0) ? TRIANGLE_INDEX_FRONT : TRIANGLE_INDEX_BACK;

                uint global_index_count;
                InterlockedAdd(results[0].indexCount, 6, global_index_count);

                [unroll]
                for (uint index = 0; index < 6; ++index) {
                    indices[global_index_count + index] = vertex_index[triangle_index[index]];
                }
            }
        }
    }
}