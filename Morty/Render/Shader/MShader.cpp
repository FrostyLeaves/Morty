#include "MShader.h"
#include "Mesh/MVertex.h"
#include "RHI/Abstract/MIDevice.h"

using namespace morty;

bool MShader::CompileShader(MIDevice* pDevice)
{
    if (false == pDevice->CompileShader(this)) return false;

    return true;
}

void MShader::CleanShader(MIDevice* pDevice) { pDevice->CleanShader(this); }
