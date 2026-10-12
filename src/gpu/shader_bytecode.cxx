#include "gpu/shader_bytecode.hxx"

#include "shaders/ui_ps.hxx"
#include "shaders/ui_vs.hxx"

namespace loader::gpu::shaders
{

ShaderBytecode uiVertex()
{
    return {g_uiVertexShader};
}

ShaderBytecode uiPixel()
{
    return {g_uiPixelShader};
}

}
