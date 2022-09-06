#pragma once

#include <list>
#include <metal-cpp-beta/metal-cpp-beta.h>
#include <string>
#include <unordered_map>

class ImGuiContext;
union SDL_Event;
namespace renderer::mtl::windowing {
struct SdlCoreData;
}

namespace renderer::mtl::rendering {

struct RenderPipeline
{
    MTL::RenderPipelineDescriptor* descriptor = nullptr;
    MTL::RenderPipelineState*      state      = nullptr;
};

struct FrameBuffer
{
    MTL::TextureDescriptor*    textureDescriptor = nullptr;
    MTL::Texture*              texture           = nullptr;
    MTL::RenderPassDescriptor* passDescriptor    = nullptr;
    MTL::ClearColor            clearColor        = MTL::ClearColor();
};

struct GBufferInfo
{
    std::string                  name           = {};
    std::unique_ptr<FrameBuffer> frameBuffer    = nullptr;
    RenderPipeline*              renderPipeline = nullptr;
};

struct GBuffer
{
    std::list<std::unique_ptr<GBufferInfo>> buffers = {};
};

struct FramePipeline
{
    std::unique_ptr<GBuffer>     gBuffer    = nullptr;
    std::unique_ptr<FrameBuffer> mainBuffer = nullptr;
};

struct MetalCoreData
{
    MTL::Device*                    device                    = nullptr;
    CA::MetalLayer*                 layer                     = nullptr;
    CA::MetalDrawable*              lastDrawable              = nullptr;
    CA::MetalDrawable*              drawable                  = nullptr;
    std::unique_ptr<FramePipeline>  framePipeline             = nullptr;
    MTL::CommandQueue*              commandQueue              = nullptr;
    MTL::RenderPassDescriptor*      imGuiRenderPassDescriptor = nullptr;
    std::unique_ptr<RenderPipeline> positionRenderPipeline    = nullptr;
    std::unique_ptr<RenderPipeline> colorRenderPipeline       = nullptr;
    std::unique_ptr<RenderPipeline> deferredRenderPipeline    = nullptr;
};

bool
init(MetalCoreData& metalCoreData, windowing::SdlCoreData& sdlCoreData);

void
deInit(MetalCoreData& metalCoreData);

void
render(MetalCoreData& metalCoreData);

void
onSDLEvent(SDL_Event* event);

void
onWindowResize(MetalCoreData& metalCoreData, int width, int height);

namespace internal {

void
renderGBuffer(MetalCoreData& metalCoreData, MTL::CommandBuffer* commandBuffer);

void
renderUI(MetalCoreData& metalCoreData, MTL::CommandBuffer* commandBuffer);

void
createMainRenderPass(MetalCoreData& metalCoreData, windowing::SdlCoreData& sdlCoreData);

void
destroyMainRenderPass(MetalCoreData& metalCoreData);

void
createUIRenderPass(MetalCoreData& metalCoreData, windowing::SdlCoreData& sdlCoreData);

void
destroyUIRenderPass(MetalCoreData& metalCoreData);

void
createRenderPipeline(RenderPipeline& renderPipeline, MTL::Device* device, const std::string& name);

void
destroyRenderPipeline(RenderPipeline& renderPipeline, MTL::Device* device);

void
createFramebufferTexture(FrameBuffer& framebuffer, MTL::Device* device, uint32_t width, uint32_t height);

void
destroyFramebufferTexture(FrameBuffer& framebuffer, MTL::Device* device);

void
reCreateFramebufferTexture(FrameBuffer& framebuffer, MTL::Device* device, uint32_t width, uint32_t height);

} // namespace internal

} // renderer::mtl::::rendering