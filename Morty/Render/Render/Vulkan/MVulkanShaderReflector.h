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

#include "Shader/MShader.h"

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


	std::tuple<VkFormat, uint32_t> GetVertexInputDescription(const std::string& name, spirv_cross::SPIRType type) const;

	void GetVertexInputState(const spirv_cross::Compiler& compiler, MVertexShaderBuffer* pShaderBuffer) const;

	void GetShaderParam(const spirv_cross::Compiler& compiler, MShaderBuffer* pShaderBuffer);
	
	void BuildVariant(const spirv_cross::Compiler& compiler, const spirv_cross::SPIRType& type, MVariant& variant);

	bool BuildBasicVariant(const spirv_cross::SPIRType& type, MVariant& variant) const;

private:
	MVulkanDevice* m_pDevice;
};


#endif
