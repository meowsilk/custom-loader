set(sdk_root "$ENV{ProgramFiles\(x86\)}/Windows Kits/10")
file(GLOB sdk_bin_dirs "${sdk_root}/bin/*/x64")
list(SORT sdk_bin_dirs COMPARE NATURAL ORDER DESCENDING)
find_program(LOADER_FXC fxc HINTS "${sdk_root}/bin/${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION}/x64" ${sdk_bin_dirs} REQUIRED)

set(shader_output_dir "${CMAKE_BINARY_DIR}/generated/shaders")

set(shader_programs
    "ui.hlsl|vsMain|vs_5_0|g_uiVertexShader|ui_vs"
    "ui.hlsl|psMain|ps_5_0|g_uiPixelShader|ui_ps"
)

file(GLOB shader_includes CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.hlsli")

set(shader_headers)
foreach(program IN LISTS shader_programs)
    string(REPLACE "|" ";" fields "${program}")
    list(GET fields 0 source_name)
    list(GET fields 1 entry)
    list(GET fields 2 profile)
    list(GET fields 3 variable)
    list(GET fields 4 output_name)

    set(source "${CMAKE_CURRENT_SOURCE_DIR}/${source_name}")
    set(header "${shader_output_dir}/${output_name}.hxx")

    add_custom_command(
        OUTPUT "${header}"
        COMMAND "${CMAKE_COMMAND}" -E make_directory "${shader_output_dir}"
        COMMAND "${LOADER_FXC}" /nologo /WX /O3 /Qstrip_debug /Qstrip_reflect
                /T ${profile} /E ${entry} /Vn ${variable} /Fh "${header}" "${source}"
        DEPENDS "${source}" ${shader_includes}
        COMMENT "Compiling ${source_name}:${entry} (${profile})"
        VERBATIM
    )
    list(APPEND shader_headers "${header}")
endforeach()

add_custom_target(loader_shaders DEPENDS ${shader_headers})
set_target_properties(loader_shaders PROPERTIES FOLDER "shaders")

add_library(loader_shader_headers INTERFACE)
target_include_directories(loader_shader_headers INTERFACE "${CMAKE_BINARY_DIR}/generated")
add_dependencies(loader_shader_headers loader_shaders)
