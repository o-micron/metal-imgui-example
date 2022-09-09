#include "renderer.h"
#include "window.h"

int
main(int, char**)
{
    NS::AutoreleasePool* pool = NS::AutoreleasePool::alloc()->init();
    renderer::mtl::windowing::SdlCoreData   sdlCoreData   = {};
    renderer::mtl::rendering::MetalCoreData metalCoreData = {};
    if (renderer::mtl::windowing::init(sdlCoreData, metalCoreData)) {
        if (renderer::mtl::rendering::init(metalCoreData, sdlCoreData)) {
            while (true) {
                NS::AutoreleasePool* pool = NS::AutoreleasePool::alloc()->init();
                if (renderer::mtl::windowing::poll(sdlCoreData, metalCoreData)) {
                    if(renderer::mtl::windowing::update(sdlCoreData, metalCoreData)) {
                        renderer::mtl::rendering::render(metalCoreData);
                    }
                } else {
                    pool->release();
                    break;
                }
                pool->release();
            }
            renderer::mtl::rendering::deInit(metalCoreData);
        }
        renderer::mtl::windowing::deInit(sdlCoreData);
    }
    pool->release();
    return 0;
}
