#include "window.h"
#include "renderer.h"

#include <SDL.h>

namespace renderer::mtl::windowing {

bool
init(SdlCoreData& sdlCoreData, renderer::mtl::rendering::MetalCoreData& metalCoreData)
{
    sdlCoreData.state = 0;

    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        printf("[FATAL] Failed to initialize window\n%s\n", SDL_GetError());
        return false;
    }

    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "metal");

    sdlCoreData.window = SDL_CreateWindow("metal-imgui-example",
                                          SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED,
                                          640,
                                          480,
                                          SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_SHOWN);

    if (!sdlCoreData.window) {
        printf("[FATAL] Failed to create a window\n%s\n", SDL_GetError());
        return false;
    }

    sdlCoreData.renderer =
      SDL_CreateRenderer(sdlCoreData.window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    if (!sdlCoreData.renderer) {
        printf("[FATAL] Failed to create renderer\n%s\n", SDL_GetError());
        return false;
    }

    metalCoreData.layer = (CA::MetalLayer*)SDL_RenderGetMetalLayer(sdlCoreData.renderer);
    if (!metalCoreData.layer) {
        printf("[FATAL] Failed to get a valid metal layer\n");
        return false;
    }
    metalCoreData.layer->setPixelFormat(MTL::PixelFormatBGRA8Unorm);

    metalCoreData.device = metalCoreData.layer->device();
    if (!metalCoreData.device) {
        printf("[FATAL] Failed to get a valid metal device\n");
        return false;
    }

    metalCoreData.drawable = metalCoreData.layer->nextDrawable();

    return true;
}

void
deInit(SdlCoreData& sdlCoreData)
{
    SDL_DestroyRenderer(sdlCoreData.renderer);
    SDL_DestroyWindow(sdlCoreData.window);
    SDL_Quit();
}

bool
poll(SdlCoreData& sdlCoreData, renderer::mtl::rendering::MetalCoreData& metalCoreData)
{
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            sdlCoreData.state |= WINDOW_STATE_SHOULD_QUIT;
            return false;
        }
        if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_FOCUS_LOST) {
            sdlCoreData.state |= WINDOW_STATE_LOST_FOCUS;
            return true;
        }
        if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_FOCUS_GAINED) {
            sdlCoreData.state &= ~WINDOW_STATE_LOST_FOCUS;
            return true;
        }
        if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_RESIZED) {
            int w, h;
            SDL_GetWindowSize(sdlCoreData.window, &w, &h);
            renderer::mtl::rendering::onWindowResize(metalCoreData, w, h);
        }
        renderer::mtl::rendering::onSDLEvent(&event);
    }

    return true;
}

bool
update(SdlCoreData& sdlCoreData, rendering::MetalCoreData& metalCoreData)
{
    if ((sdlCoreData.state & WINDOW_STATE_LOST_FOCUS) == WINDOW_STATE_LOST_FOCUS) { return false; }

    int width, height;
    SDL_GetRendererOutputSize(sdlCoreData.renderer, &width, &height);
    metalCoreData.layer->setDrawableSize(CGSizeMake(width, height));
    metalCoreData.drawable = metalCoreData.layer->nextDrawable();
    if (!metalCoreData.drawable) { printf("[FATAL] Failed to get a valid metal drawable\n"); }

    return true;
}

} // renderer::mtl::windowing
