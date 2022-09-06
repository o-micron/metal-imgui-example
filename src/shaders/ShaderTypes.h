#pragma once

#include <simd/simd.h>

namespace renderer::mtl::shading {

typedef enum VertexInputIndex
{
    VertexInputIndexVertices     = 0,
    VertexInputIndexViewportSize = 1,
} VertexInputIndex;

typedef enum TextureIndex
{
    TextureIndexPositionColor = 0,
    TextureIndexAlbedoColor   = 1,
} TextureIndex;

typedef struct
{
    vector_float2 position;
    vector_float2 coord;
    vector_float3 color;
} Vertex;

} // namespace renderer::mtl::shading