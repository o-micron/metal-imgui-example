#include "renderer.h"
#include "window.h"

bool
cycle(renderer::mtl::windowing::SdlCoreData& sdlCoreData, renderer::mtl::rendering::MetalCoreData& metalCoreData)
{
    NS::AutoreleasePool* loopPool = NS::AutoreleasePool::alloc()->init();
    if (renderer::mtl::windowing::poll(sdlCoreData, metalCoreData)) {
        if (renderer::mtl::windowing::update(sdlCoreData, metalCoreData)) {
            renderer::mtl::rendering::render(metalCoreData);
        }
    } else {
        loopPool->release();
        return false;
    }
    loopPool->release();
    return true;
}

int
main(int argc, char** argv)
{
    renderer::mtl::windowing::SdlCoreData   sdlCoreData   = {};
    renderer::mtl::rendering::MetalCoreData metalCoreData = {};
    if (renderer::mtl::windowing::init(sdlCoreData, metalCoreData)) {
        if (renderer::mtl::rendering::init(metalCoreData, sdlCoreData)) {
            if (argc == 1) {
                // infinite loop
                while (true) {
                    if (!cycle(sdlCoreData, metalCoreData)) { break; }
                }
            } else if (argc == 2) {
                // mainly used for testing, to render a couple of frames only
                char* p;
                int   cyclesCount = strtol(argv[1], &p, 10);
                for (int i = 0; i < cyclesCount; ++i) { cycle(sdlCoreData, metalCoreData); }
            }
            renderer::mtl::rendering::deInit(metalCoreData);
        }
        renderer::mtl::windowing::deInit(sdlCoreData);
    }
    return 0;
}
