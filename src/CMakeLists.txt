file(GLOB_RECURSE loader_sources CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cxx" "${CMAKE_CURRENT_SOURCE_DIR}/*.hxx")
list(FILTER loader_sources EXCLUDE REGEX "/main\\.cxx$")
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${loader_sources})

function(loader_apply_build_settings target)
    target_compile_definitions(${target} PRIVATE
        WIN32_LEAN_AND_MEAN NOMINMAX UNICODE _UNICODE
        WINVER=0x0A00 _WIN32_WINNT=0x0A00)
    target_compile_options(${target} PRIVATE /W4 /WX /permissive- /utf-8 /Zc:__cplusplus /Zc:preprocessor /MP)
    if(LOADER_ENABLE_LTO)
        set_property(TARGET ${target} PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)
    endif()
endfunction()

add_library(loader_framework STATIC ${loader_sources})
target_include_directories(loader_framework PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(loader_framework PUBLIC
    freetype harfbuzz msdfgen::msdfgen-core
    loader_shader_headers
    d3d11 dxgi dxguid dcomp shcore imm32 crypt32 windowsapp windowscodecs ole32)
loader_apply_build_settings(loader_framework)

add_executable(loader_gg WIN32 main.cxx app/app.manifest $<TARGET_OBJECTS:loader_assets>)
set_target_properties(loader_gg PROPERTIES OUTPUT_NAME "loader.gg")
target_link_libraries(loader_gg PRIVATE loader_framework)
loader_apply_build_settings(loader_gg)
