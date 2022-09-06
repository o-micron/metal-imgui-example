#include "renderer.h"
#include "window.h"

int
main(int, char**)
{
    renderer::mtl::windowing::SdlCoreData   sdlCoreData   = {};
    renderer::mtl::rendering::MetalCoreData metalCoreData = {};
    if (renderer::mtl::windowing::init(sdlCoreData, metalCoreData)) {
        if (renderer::mtl::rendering::init(metalCoreData, sdlCoreData)) {
            while (true) {
                if (renderer::mtl::windowing::poll(sdlCoreData, metalCoreData)) {
                    renderer::mtl::windowing::update(sdlCoreData, metalCoreData);
                    renderer::mtl::rendering::render(metalCoreData);
                } else {
                    break;
                }
            }
            renderer::mtl::rendering::deInit(metalCoreData);
        }
        renderer::mtl::windowing::deInit(sdlCoreData);
    }
    return 0;
}
