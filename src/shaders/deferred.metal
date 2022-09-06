#include <metal_stdlib>

using namespace metal;

#include "ShaderTypes.h"

struct RasterizerData
{
    float4 position [[position]];
    float2 coord;
    float3 color;
};

vertex RasterizerData
vertexShader(uint vertexID [[vertex_id]],
             constant renderer::mtl::shading::Vertex *vertices [[buffer(renderer::mtl::shading::VertexInputIndexVertices)]],
             constant vector_uint2 *viewportSizePointer [[buffer(renderer::mtl::shading::VertexInputIndexViewportSize)]])
{
    RasterizerData out;
    out.position = vector_float4(vertices[vertexID].position, 0.0, 1.0);
    out.coord = vertices[vertexID].coord;
    out.color = vertices[vertexID].color;
    return out;
}

fragment float4 fragmentShader(RasterizerData in [[stage_in]],
                               texture2d<half> positionTexture [[ texture(renderer::mtl::shading::TextureIndexPositionColor) ]],
                               texture2d<half> albedoTexture [[ texture(renderer::mtl::shading::TextureIndexAlbedoColor) ]])
{
    constexpr sampler textureSampler (mag_filter::linear, min_filter::linear);

    // Sample the texture to obtain a color
    const half4 positionSample = positionTexture.sample(textureSampler, in.coord);
    const half4 albedoSample = albedoTexture.sample(textureSampler, in.coord);

    // return the color of the texture
    return float4(albedoSample) * float4(positionSample);
}
