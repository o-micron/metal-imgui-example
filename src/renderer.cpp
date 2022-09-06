#include "renderer.h"
#include "shaders/ShaderTypes.h"
#include "window.h"

#include <array>
#include <filesystem>
#include <stdio.h>

#include <SDL.h>

#include <imgui.h>
#include <imgui_impl_metal.h>
#include <imgui_impl_sdl.h>

#define NS_STRING_FROM_CSTRING(STR) NS::String::string(STR, NS::UTF8StringEncoding)

#define METAL_CHECK(PTR, MSG)                                                                                          \
    if (!PTR) { printf(MSG); }

namespace renderer::mtl::rendering {

bool
init(MetalCoreData& metalCoreData, windowing::SdlCoreData& sdlCoreData)
{
    metalCoreData.framePipeline             = std::make_unique<FramePipeline>();
    metalCoreData.framePipeline->mainBuffer = std::make_unique<FrameBuffer>();
    metalCoreData.framePipeline->gBuffer    = std::make_unique<GBuffer>();

    metalCoreData.colorRenderPipeline    = std::make_unique<RenderPipeline>();
    metalCoreData.positionRenderPipeline = std::make_unique<RenderPipeline>();
    metalCoreData.deferredRenderPipeline = std::make_unique<RenderPipeline>();

    metalCoreData.commandQueue = metalCoreData.device->newCommandQueue();

    internal::createRenderPipeline(*metalCoreData.colorRenderPipeline, metalCoreData.device, "shaders/color.metallib");
    internal::createRenderPipeline(
      *metalCoreData.positionRenderPipeline, metalCoreData.device, "shaders/position.metallib");
    internal::createRenderPipeline(
      *metalCoreData.deferredRenderPipeline, metalCoreData.device, "shaders/deferred.metallib");

    internal::createMainRenderPass(metalCoreData, sdlCoreData);
    internal::createUIRenderPass(metalCoreData, sdlCoreData);

    return true;
}

void
deInit(MetalCoreData& metalCoreData)
{
    internal::destroyRenderPipeline(*metalCoreData.deferredRenderPipeline, metalCoreData.device);
    internal::destroyRenderPipeline(*metalCoreData.positionRenderPipeline, metalCoreData.device);
    internal::destroyRenderPipeline(*metalCoreData.colorRenderPipeline, metalCoreData.device);
    internal::destroyUIRenderPass(metalCoreData);
    internal::destroyMainRenderPass(metalCoreData);
    metalCoreData.commandQueue->release();
}

void
render(MetalCoreData& metalCoreData)
{
    metalCoreData.framePipeline->mainBuffer->passDescriptor->colorAttachments()->object(0)->setClearColor(
      metalCoreData.framePipeline->mainBuffer->clearColor);
    metalCoreData.framePipeline->mainBuffer->passDescriptor->colorAttachments()->object(0)->setLoadAction(
      MTL::LoadActionClear);
    metalCoreData.framePipeline->mainBuffer->passDescriptor->colorAttachments()->object(0)->setStoreAction(
      MTL::StoreActionStore);
    metalCoreData.framePipeline->mainBuffer->passDescriptor->colorAttachments()->object(0)->setTexture(
      metalCoreData.framePipeline->mainBuffer->texture);

    MTL::ClearColor uiClearColor(0.2f, 0.0f, 0.0f, 1.0);
    metalCoreData.imGuiRenderPassDescriptor->colorAttachments()->object(0)->setClearColor(uiClearColor);
    metalCoreData.imGuiRenderPassDescriptor->colorAttachments()->object(0)->setLoadAction(MTL::LoadActionClear);
    metalCoreData.imGuiRenderPassDescriptor->colorAttachments()->object(0)->setStoreAction(MTL::StoreActionStore);
    metalCoreData.imGuiRenderPassDescriptor->colorAttachments()->object(0)->setTexture(
      metalCoreData.lastDrawable->texture());

    MTL::CommandBuffer* commandBuffer = metalCoreData.commandQueue->commandBuffer();

    internal::renderGBuffer(metalCoreData, commandBuffer);
    internal::renderUI(metalCoreData, commandBuffer);

    // Schedule a present once the framebuffer is complete using the current drawable.
    commandBuffer->presentDrawable(metalCoreData.drawable);

    // Finalize rendering here & push the command buffer to the GPU.
    commandBuffer->commit();
}

void
onSDLEvent(SDL_Event* event)
{
    ImGui_ImplSDL2_ProcessEvent(event);
}

void
onWindowResize(MetalCoreData& metalCoreData, int width, int height)
{
    for (const auto& buffer : metalCoreData.framePipeline->gBuffer->buffers) {
        buffer->frameBuffer->textureDescriptor->setWidth(width);
        buffer->frameBuffer->textureDescriptor->setHeight(height);
        internal::reCreateFramebufferTexture(*buffer->frameBuffer, metalCoreData.device, width, height);
    }
    metalCoreData.framePipeline->mainBuffer->textureDescriptor->setWidth(width);
    metalCoreData.framePipeline->mainBuffer->textureDescriptor->setHeight(height);
    internal::reCreateFramebufferTexture(*metalCoreData.framePipeline->mainBuffer, metalCoreData.device, width, height);
}

namespace internal {

void
renderGBuffer(MetalCoreData& metalCoreData, MTL::CommandBuffer* commandBuffer)
{
    static const shading::Vertex triangleVertices[] = {
        { .position = { 0.5, 0.5 }, .coord{ 1.0, 0.0 }, .color = { 1.0, 0.0, 0.0 } },
        { .position = { -0.5, 0.5 }, .coord{ 0.0, 0.0 }, .color = { 0.0, 1.0, 0.0 } },
        { .position = { 0.0, -0.5 }, .coord{ 1.0, 1.0 }, .color = { 0.0, 0.0, 1.0 } },
    };

    static const shading::Vertex quadVertices[] = {
        { .position = { 1.0, -1.0 }, .coord{ 1.0, 1.0 }, .color = { 1.0, 0.0, 0.0 } },
        { .position = { -1.0, -1.0 }, .coord{ 0.0, 1.0 }, .color = { 0.0, 1.0, 0.0 } },
        { .position = { -1.0, 1.0 }, .coord{ 0.0, 0.0 }, .color = { 0.0, 0.0, 1.0 } },
        { .position = { -1.0, 1.0 }, .coord{ 0.0, 0.0 }, .color = { 0.0, 0.0, 1.0 } },
        { .position = { 1.0, 1.0 }, .coord{ 1.0, 0.0 }, .color = { 0.0, 1.0, 1.0 } },
        { .position = { 1.0, -1.0 }, .coord{ 1.0, 1.0 }, .color = { 1.0, 1.0, 0.0 } },
    };

    commandBuffer->setLabel(NS_STRING_FROM_CSTRING("MainCommandBuffer"));

    for (const auto& buffer : metalCoreData.framePipeline->gBuffer->buffers) {
        std::array<uint32_t, 2> dimensions = {
            static_cast<uint32_t>(metalCoreData.framePipeline->mainBuffer->texture->width()),
            static_cast<uint32_t>(metalCoreData.framePipeline->mainBuffer->texture->height())
        };
        MTL::Viewport viewport = {};
        viewport.originX       = 0.0;
        viewport.originY       = 0.0;
        viewport.width         = dimensions[0];
        viewport.height        = dimensions[1];
        viewport.znear         = 0.0;
        viewport.zfar          = 1.0;

        buffer->frameBuffer->passDescriptor->colorAttachments()->object(0)->setClearColor(
          buffer->frameBuffer->clearColor);
        buffer->frameBuffer->passDescriptor->colorAttachments()->object(0)->setLoadAction(MTL::LoadActionClear);
        buffer->frameBuffer->passDescriptor->colorAttachments()->object(0)->setStoreAction(MTL::StoreActionStore);
        buffer->frameBuffer->passDescriptor->colorAttachments()->object(0)->setTexture(buffer->frameBuffer->texture);

        auto encoder = commandBuffer->renderCommandEncoder(buffer->frameBuffer->passDescriptor);
        auto label   = NS_STRING_FROM_CSTRING(buffer->name.c_str());
        encoder->setLabel(label);
        encoder->pushDebugGroup(label);
        encoder->setViewport(viewport);
        encoder->setRenderPipelineState(buffer->renderPipeline->state);
        encoder->setVertexBytes(triangleVertices, sizeof(triangleVertices), shading::VertexInputIndexVertices);
        encoder->setVertexBytes(&dimensions, sizeof(dimensions), shading::VertexInputIndexViewportSize);
        encoder->drawPrimitives(MTL::PrimitiveTypeTriangle, NS::UInteger(0), NS::UInteger(3));
        encoder->popDebugGroup();
        encoder->endEncoding();
    }

    {
        std::array<uint32_t, 2> dimensions = {
            static_cast<uint32_t>(metalCoreData.framePipeline->mainBuffer->texture->width()),
            static_cast<uint32_t>(metalCoreData.framePipeline->mainBuffer->texture->height())
        };
        MTL::Viewport viewport = {};
        viewport.originX       = 0.0;
        viewport.originY       = 0.0;
        viewport.width         = dimensions[0];
        viewport.height        = dimensions[1];
        viewport.znear         = 0.0;
        viewport.zfar          = 1.0;

        auto encoder = commandBuffer->renderCommandEncoder(metalCoreData.framePipeline->mainBuffer->passDescriptor);
        encoder->setLabel(NS_STRING_FROM_CSTRING("MainRenderEncoder"));
        encoder->pushDebugGroup(NS_STRING_FROM_CSTRING("MainRenderEncoder"));
        encoder->setViewport(viewport);
        encoder->setRenderPipelineState(metalCoreData.deferredRenderPipeline->state);
        encoder->setVertexBytes(quadVertices, sizeof(quadVertices), shading::VertexInputIndexVertices);
        encoder->setVertexBytes(&dimensions, sizeof(dimensions), shading::VertexInputIndexViewportSize);
        uint32_t index = 0;
        for (const auto& buffer : metalCoreData.framePipeline->gBuffer->buffers) {
            encoder->setFragmentTexture(buffer->frameBuffer->texture, index++);
        }
        encoder->drawPrimitives(MTL::PrimitiveTypeTriangle, NS::UInteger(0), NS::UInteger(6));
        encoder->popDebugGroup();
        encoder->endEncoding();
    }
}

void
renderUI(MetalCoreData& metalCoreData, MTL::CommandBuffer* commandBuffer)
{
    MTL::RenderCommandEncoder* encoder = commandBuffer->renderCommandEncoder(metalCoreData.imGuiRenderPassDescriptor);
    encoder->setLabel(NS_STRING_FROM_CSTRING("UIRenderEncoder"));
    encoder->pushDebugGroup(NS_STRING_FROM_CSTRING("UIRenderEncoder"));

    ImGui_ImplMetal_NewFrame(metalCoreData.imGuiRenderPassDescriptor);
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    if (ImGui::Begin("GBuffer", (bool*)0, ImGuiWindowFlags_AlwaysVerticalScrollbar)) {
        for (const auto& buffer : metalCoreData.framePipeline->gBuffer->buffers) {
            ImGui::Text("%s frame buffer", buffer->name.c_str());
            ImGui::Image(buffer->frameBuffer->texture, ImVec2(300, 300));
        }
        ImGui::Text("Main frame buffer");
        ImGui::Image(metalCoreData.framePipeline->mainBuffer->texture, ImVec2(300, 300));
    }
    ImGui::End();

    ImGui::EndFrame();
    ImGui::Render();
    ImGui_ImplMetal_RenderDrawData(ImGui::GetDrawData(), commandBuffer, encoder);

    encoder->popDebugGroup();
    encoder->endEncoding();
}

void
createMainRenderPass(MetalCoreData& metalCoreData, windowing::SdlCoreData& sdlCoreData)
{
    int width, height;
    SDL_GetRendererOutputSize(sdlCoreData.renderer, &width, &height);

    metalCoreData.framePipeline->mainBuffer                 = std::make_unique<FrameBuffer>();
    metalCoreData.framePipeline->mainBuffer->passDescriptor = MTL::RenderPassDescriptor::alloc()->init();
    METAL_CHECK(metalCoreData.framePipeline->mainBuffer->passDescriptor,
                "Failed to get a valid metal render pass descriptor");
    createFramebufferTexture(*metalCoreData.framePipeline->mainBuffer.get(), metalCoreData.device, width, height);
    metalCoreData.framePipeline->mainBuffer->clearColor = MTL::ClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    metalCoreData.framePipeline->gBuffer = std::make_unique<GBuffer>();

    // position
    {
        auto gBufferPositionFrameBuffer            = std::make_unique<FrameBuffer>();
        gBufferPositionFrameBuffer->passDescriptor = MTL::RenderPassDescriptor::alloc()->init();
        METAL_CHECK(gBufferPositionFrameBuffer->passDescriptor, "Failed to get a valid metal render pass descriptor");
        createFramebufferTexture(*gBufferPositionFrameBuffer.get(), metalCoreData.device, width, height);
        gBufferPositionFrameBuffer->clearColor = MTL::ClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        auto positionGBufferInfo               = std::make_unique<GBufferInfo>();
        positionGBufferInfo->name              = "Position";
        positionGBufferInfo->frameBuffer       = std::move(gBufferPositionFrameBuffer);
        positionGBufferInfo->renderPipeline    = &(*metalCoreData.positionRenderPipeline);
        metalCoreData.framePipeline->gBuffer->buffers.push_front(std::move(positionGBufferInfo));
    }
    // color
    {
        auto gBufferColorFrameBuffer            = std::make_unique<FrameBuffer>();
        gBufferColorFrameBuffer->passDescriptor = MTL::RenderPassDescriptor::alloc()->init();
        METAL_CHECK(gBufferColorFrameBuffer->passDescriptor, "Failed to get a valid metal render pass descriptor");
        createFramebufferTexture(*gBufferColorFrameBuffer.get(), metalCoreData.device, width, height);
        gBufferColorFrameBuffer->clearColor = MTL::ClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        auto colorGBufferInfo               = std::make_unique<GBufferInfo>();
        colorGBufferInfo->name              = "Color";
        colorGBufferInfo->frameBuffer       = std::move(gBufferColorFrameBuffer);
        colorGBufferInfo->renderPipeline    = &(*metalCoreData.colorRenderPipeline);
        metalCoreData.framePipeline->gBuffer->buffers.push_front(std::move(colorGBufferInfo));
    }
}

void
destroyMainRenderPass(MetalCoreData& metalCoreData)
{
    destroyFramebufferTexture(*metalCoreData.framePipeline->mainBuffer, metalCoreData.device);
    metalCoreData.framePipeline->mainBuffer->passDescriptor->release();
    for (const auto& buffer : metalCoreData.framePipeline->gBuffer->buffers) {
        destroyFramebufferTexture(*buffer->frameBuffer, metalCoreData.device);
        buffer->frameBuffer->passDescriptor->release();
    }
}

void
createUIRenderPass(MetalCoreData& metalCoreData, windowing::SdlCoreData& sdlCoreData)
{
    metalCoreData.imGuiRenderPassDescriptor = MTL::RenderPassDescriptor::alloc()->init();
    METAL_CHECK(metalCoreData.imGuiRenderPassDescriptor, "Failed to get a valid metal render pass descriptor");

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls

    // Setup style
    ImGui::StyleColorsDark();

    ImGuiStyle& style = ImGui::GetStyle();

    style.AntiAliasedLines           = true;
    style.AntiAliasedLinesUseTex     = true;
    style.AntiAliasedFill            = true;
    style.ChildRounding              = 5.0f;
    style.CircleTessellationMaxError = 0.1f;
    style.CurveTessellationTol       = 0.1f;
    style.FrameRounding              = 0.0f;
    style.GrabRounding               = 5.0f;
    style.ScrollbarRounding          = 0.0f;
    style.ScrollbarSize              = 4.0f;
    style.TabBorderSize              = 0.0f;
    style.TabRounding                = 0.0f;
    style.WindowRounding             = 5.0f;
    style.WindowTitleAlign           = ImVec2(0.01f, 0.5f);
    style.PopupRounding              = 5.0f;

    ImGui_ImplMetal_Init(metalCoreData.device);
    ImGui_ImplSDL2_InitForMetal(sdlCoreData.window);
}

void
destroyUIRenderPass(MetalCoreData& metalCoreData)
{
    metalCoreData.imGuiRenderPassDescriptor->release();
    ImGui_ImplMetal_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
}

void
createRenderPipeline(RenderPipeline& renderPipeline, MTL::Device* device, const std::string& name)
{
    auto                  executablePath = NS::Bundle::mainBundle()->executablePath();
    std::filesystem::path fsExecutablePath(executablePath->cString(NS::UTF8StringEncoding));
    auto                  fsMetalLibPath = fsExecutablePath.parent_path() / name;
    auto                  metalLibPath   = NS_STRING_FROM_CSTRING(fsMetalLibPath.string().c_str());
    NS::Error*            error;
    auto                  defaultLibrary = NS::TransferPtr(device->newLibrary(metalLibPath, &error));
    METAL_CHECK(defaultLibrary.get(), "Failed to create new library from metal lib");
    auto vertexFunction   = NS::TransferPtr(defaultLibrary->newFunction(NS_STRING_FROM_CSTRING("vertexShader")));
    auto fragmentFunction = NS::TransferPtr(defaultLibrary->newFunction(NS_STRING_FROM_CSTRING("fragmentShader")));

    // Configure a pipeline descriptor that is used to create a pipeline state.
    renderPipeline.descriptor = MTL::RenderPipelineDescriptor::alloc()->init();
    renderPipeline.descriptor->setLabel(NS_STRING_FROM_CSTRING(name.c_str()));
    renderPipeline.descriptor->setVertexFunction(vertexFunction.get());
    renderPipeline.descriptor->setFragmentFunction(fragmentFunction.get());
    renderPipeline.descriptor->colorAttachments()->object(0)->setPixelFormat(MTL::PixelFormatBGRA8Unorm);

    renderPipeline.state = device->newRenderPipelineState(renderPipeline.descriptor, &error);
    METAL_CHECK(renderPipeline.state, "Failed to create a metal render pipeline state");
}

void
destroyRenderPipeline(RenderPipeline& renderPipeline, MTL::Device* device)
{
    renderPipeline.descriptor->release();
    renderPipeline.state->release();
}

void
createFramebufferTexture(FrameBuffer& framebuffer, MTL::Device* device, uint32_t width, uint32_t height)
{
    framebuffer.textureDescriptor = MTL::TextureDescriptor::alloc()->init();
    METAL_CHECK(framebuffer.textureDescriptor, "Failed to create a texture descriptor");
    framebuffer.textureDescriptor->setWidth(width);
    framebuffer.textureDescriptor->setHeight(height);
    framebuffer.textureDescriptor->setPixelFormat(MTL::PixelFormatBGRA8Unorm);
    framebuffer.textureDescriptor->setStorageMode(MTL::StorageModePrivate);
    framebuffer.textureDescriptor->setUsage(MTL::TextureUsageRenderTarget | MTL::TextureUsageShaderRead);

    framebuffer.texture = device->newTexture(framebuffer.textureDescriptor);
    METAL_CHECK(framebuffer.texture, "Failed to create a texture");
}

void
destroyFramebufferTexture(FrameBuffer& framebuffer, MTL::Device* device)
{
    framebuffer.textureDescriptor->release();
    framebuffer.texture->release();
}

void
reCreateFramebufferTexture(FrameBuffer& framebuffer, MTL::Device* device, uint32_t width, uint32_t height)
{
    destroyFramebufferTexture(framebuffer, device);
    createFramebufferTexture(framebuffer, device, width, height);
}

} // namespace internal

} // renderer::mtl::rendering