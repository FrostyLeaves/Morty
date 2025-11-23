#include "IShaderProgram.h"
#include "Engine/MEngine.h"
#include "MShaderBuffer.h"
#include "RHI/Abstract/MIDevice.h"
#include "Resource/MMaterialResource.h"
#include "Resource/MShaderResource.h"
#include "Resource/MTextureResource.h"
#include "Shader/MShader.h"
#include "System/MRenderSystem.h"
#include "System/MResourceSystem.h"
#include "Utility/MFileHelper.h"
#include "Variant/MVariant.h"

using namespace morty;

MORTY_INTERFACE_IMPLEMENT(IShaderProgram, MTypeClass)
