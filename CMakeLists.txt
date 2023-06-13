cmake_minimum_required(VERSION 3.0.0)
project(metal-imgui-example VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# configure_file(${CMAKE_SOURCE_DIR}/imgui/imconfig.h ${CMAKE_SOURCE_DIR}/src/imgui/imconfig.h COPYONLY)
# configure_file(${CMAKE_SOURCE_DIR}/imgui/imgui_demo.cpp ${CMAKE_SOURCE_DIR}/src/imgui/imgui_demo.cpp COPYONLY)
# configure_file(${CMAKE_SOURCE_DIR}/imgui/imgui_draw.cpp ${CMAKE_SOURCE_DIR}/src/imgui/imgui_draw.cpp COPYONLY)
# configure_file(${CMAKE_SOURCE_DIR}/imgui/imgui_internal.h ${CMAKE_SOURCE_DIR}/src/imgui/imgui_internal.h COPYONLY)
# configure_file(${CMAKE_SOURCE_DIR}/imgui/imgui_tables.cpp ${CMAKE_SOURCE_DIR}/src/imgui/imgui_tables.cpp COPYONLY)
# configure_file(${CMAKE_SOURCE_DIR}/imgui/imgui_widgets.cpp ${CMAKE_SOURCE_DIR}/src/imgui/imgui_widgets.cpp COPYONLY)
# configure_file(${CMAKE_SOURCE_DIR}/imgui/imgui.cpp ${CMAKE_SOURCE_DIR}/src/imgui/imgui.cpp COPYONLY)
# configure_file(${CMAKE_SOURCE_DIR}/imgui/imgui.h ${CMAKE_SOURCE_DIR}/src/imgui/imgui.h COPYONLY)
# configure_file(${CMAKE_SOURCE_DIR}/imgui/imstb_rectpack.h ${CMAKE_SOURCE_DIR}/src/imgui/imstb_rectpack.h COPYONLY)
# configure_file(${CMAKE_SOURCE_DIR}/imgui/imstb_textedit.h ${CMAKE_SOURCE_DIR}/src/imgui/imstb_textedit.h COPYONLY)
# configure_file(${CMAKE_SOURCE_DIR}/imgui/imstb_truetype.h ${CMAKE_SOURCE_DIR}/src/imgui/imstb_truetype.h COPYONLY)
# configure_file(${CMAKE_SOURCE_DIR}/imgui/backends/imgui_impl_metal_cpp.h ${CMAKE_SOURCE_DIR}/src/imgui/imgui_impl_metal_cpp.h COPYONLY)
# configure_file(${CMAKE_SOURCE_DIR}/imgui/backends/imgui_impl_metal_cpp.cpp ${CMAKE_SOURCE_DIR}/src/imgui/imgui_impl_metal_cpp.cpp COPYONLY)
# configure_file(${CMAKE_SOURCE_DIR}/imgui/backends/imgui_impl_sdl.h ${CMAKE_SOURCE_DIR}/src/imgui/imgui_impl_sdl.h COPYONLY)
# configure_file(${CMAKE_SOURCE_DIR}/imgui/backends/imgui_impl_sdl.cpp ${CMAKE_SOURCE_DIR}/src/imgui/imgui_impl_sdl.cpp COPYONLY)

file(GLOB_RECURSE SOURCES ${CMAKE_SOURCE_DIR}/src/*.cpp ${CMAKE_SOURCE_DIR}/imgui/*.cpp)
file(GLOB_RECURSE HEADERS ${CMAKE_SOURCE_DIR}/src/*.h ${CMAKE_SOURCE_DIR}/imgui/*.h)

add_executable(metal-imgui-example ${SOURCES} ${HEADERS})
target_sources(metal-imgui-example PRIVATE ${SOURCES} ${HEADERS})

target_include_directories(metal-imgui-example
    PUBLIC
        ${CMAKE_SOURCE_DIR}/imgui
        ${CMAKE_SOURCE_DIR}/SDL/include
        ${CMAKE_BINARY_DIR}/SDL/include
        ${CMAKE_SOURCE_DIR}/metal-cpp-beta
)
target_link_directories(metal-imgui-example
    PUBLIC
        ${CMAKE_SOURCE_DIR}/SDL/build
)
target_link_libraries(metal-imgui-example
    PUBLIC
        $<IF:$<CONFIG:Debug>,SDL2d,SDL2>
        $<IF:$<CONFIG:Debug>,SDL2maind,SDL2main>
        iconv
        "-framework AudioToolbox"
        "-framework AVFoundation"
        "-framework Carbon"
        "-framework Cocoa"
        "-framework CoreAudio"
        "-framework CoreGraphics"
        "-framework CoreHaptics"
        "-framework CoreMotion"
        "-framework Foundation"
        "-framework ForceFeedback"
        "-framework GameController"
        "-framework ImageIO"
        "-framework IOKit"
        "-framework QuartzCore"
        "-framework Metal"
        "-framework MetalKit"
)
target_compile_definitions(metal-imgui-example
    PUBLIC
        IMGUI_IMPL_METAL_CPP=1
)

# COMPILE METAL SHADERS ---------------------------------------------------------------------------
set(MetalShaders )
set(MetalLibOutputDir $<IF:$<CONFIG:Debug>,${CMAKE_CURRENT_BINARY_DIR}/Debug/shaders/,${CMAKE_CURRENT_BINARY_DIR}/Release/shaders/>)
file(GLOB_RECURSE METAL_SHADERS ${CMAKE_SOURCE_DIR}/src/shaders/*.metal)
foreach(METAL_SHADER ${METAL_SHADERS})
    get_filename_component(METAL_SHADER_FILENAME ${METAL_SHADER} NAME)
    get_filename_component(METAL_SHADER_WITHOUT_EXTENSION ${METAL_SHADER} NAME_WE)
    message(STATUS "[METAL] [${METAL_SHADER_WITHOUT_EXTENSION}] ${METAL_SHADER_FILENAME}")
    list(APPEND MetalShaders COMMAND ${CMAKE_SOURCE_DIR}/scripts/metal/compile.sh ${METAL_SHADER} ${METAL_SHADER_WITHOUT_EXTENSION} ${MetalLibOutputDir})
endforeach()
add_custom_target(CompileShaders
    ${MetalShaders}
    WORKING_DIRECTORY  ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "[COMPILING ALL METAL SHADERS]"
)
add_dependencies(metal-imgui-example CompileShaders)
# -------------------------------------------------------------------------------------------------
