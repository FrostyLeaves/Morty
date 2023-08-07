/**
 * @File         MVulkanShaderReflector
 * 
 * @Created      2020-06-27 13:42:59
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Render/MRenderGlobal.h"

#if RENDER_GRAPHICS == MORTY_VULKAN

#include "Material/MShader.h"

#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif

#include "spirv_cross/spirv_cross.hpp"
#include "spirv_cross/spirv_parser.hpp"

class MShaderBuffer;
class MVulkanDevice;
class MVertexShaderBuffer;

class MORTY_API MVulkanShaderReflector
{
public:
	MVulkanShaderReflector(MVulkanDevice* pDevice);
	~MVulkanShaderReflector();

    bool Initialize();
	
	void GetVertexInputState(const spirv_cross::Compiler& compiler, MVertexShaderBuffer* pShaderBuffer);

	void GetComputeInputState(const spirv_cross::Compiler& compiler, MComputeShaderBuffer* pShaderBuffer);
	
	void GetShaderParam(const spirv_cross::Compiler& compiler, MShaderBuffer* pShaderBuffer);
	
	void ConvertVariant(const spirv_cross::Compiler& compiler, const spirv_cross::SPIRType& type, MVariant& variant);

	bool ResetVariantType(const spirv_cross::SPIRType& type, MVariant& variant);

private:
	MVulkanDevice* m_pDevice;
};


#endif
