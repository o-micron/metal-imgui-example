#pragma once

#include <stdint.h>

class SDL_Window;
class SDL_Renderer;
namespace renderer::mtl::rendering {
struct MetalCoreData;
}

namespace renderer::mtl::windowing {

enum SdlWindowState : uint32_t
{
    WINDOW_STATE_OKAY        = 0U,
    WINDOW_STATE_LOST_FOCUS  = 1 << 0U,
    WINDOW_STATE_SHOULD_QUIT = 1 << 1U
};

struct SdlCoreData
{
    SDL_Window*   window   = nullptr;
    SDL_Renderer* renderer = nullptr;
    uint32_t      state    = WINDOW_STATE_OKAY;
};

bool
init(SdlCoreData& sdlCoreData, rendering::MetalCoreData& metalCoreData);

void
deInit(SdlCoreData& sdlCoreData);

bool
poll(SdlCoreData& sdlCoreData, rendering::MetalCoreData& metalCoreData);

bool
update(SdlCoreData& sdlCoreData, rendering::MetalCoreData& metalCoreData);

} // namespace renderer::mtl::windowing