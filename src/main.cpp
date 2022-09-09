#include "renderer.h"
#include "window.h"

int
main(int, char**)
{
    NS::AutoreleasePool* mainPool = NS::AutoreleasePool::alloc()->init();
    renderer::mtl::windowing::SdlCoreData   sdlCoreData   = {};
    renderer::mtl::rendering::MetalCoreData metalCoreData = {};
    if (renderer::mtl::windowing::init(sdlCoreData, metalCoreData)) {
        if (renderer::mtl::rendering::init(metalCoreData, sdlCoreData)) {
            while (true) {
                NS::AutoreleasePool* loopPool = NS::AutoreleasePool::alloc()->init();
                if (renderer::mtl::windowing::poll(sdlCoreData, metalCoreData)) {
                    if(renderer::mtl::windowing::update(sdlCoreData, metalCoreData)) {
                        renderer::mtl::rendering::render(metalCoreData);
                    }
                } else {
                    loopPool->release();
                    break;
                }
                loopPool->release();
            }
            renderer::mtl::rendering::deInit(metalCoreData);
        }
        renderer::mtl::windowing::deInit(sdlCoreData);
    }
    mainPool->release();
    return 0;
}
