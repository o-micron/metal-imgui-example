#include <metal_stdlib>

using namespace metal;

#include "ShaderTypes.h"

struct RasterizerData
{
    float4 position [[position]];
    float3 color;
};

vertex RasterizerData
vertexShader(uint vertexID [[vertex_id]],
             constant renderer::mtl::shading::Vertex *vertices [[buffer(renderer::mtl::shading::VertexInputIndexVertices)]],
             constant vector_uint2 *viewportSizePointer [[buffer(renderer::mtl::shading::VertexInputIndexViewportSize)]])
{
    RasterizerData out;
    out.position = vector_float4(vertices[vertexID].position, 0.0, 1.0);
    out.color = vertices[vertexID].color;
    return out;
}

fragment float4 fragmentShader(RasterizerData in [[stage_in]])
{
    return vector_float4(in.position.xyz, 1.0);
}
