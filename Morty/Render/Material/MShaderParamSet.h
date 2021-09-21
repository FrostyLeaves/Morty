/**
 * @File         MShaderParamSet
 * 
 * @Created      2020-07-20 14:26:56
 *
 * @Author       DoubleYe
**/

#ifndef _M_MSHADERPARAMSET_H_
#define _M_MSHADERPARAMSET_H_
#include "MGlobal.h"
#include "MShaderParam.h"

#include <vector>

class MIDevice;
class MORTY_API MShaderParamSet
{
public:
	MShaderParamSet();
	MShaderParamSet(const uint32_t& unKey);

    virtual ~MShaderParamSet();

public:

	MShaderConstantParam* FindConstantParam(const MString& strParamName);


	MVariant* FindValue(const MString& strName, MVariant& value);
	MVariant* FindValue(const MString& strName);

	bool SetValue(MVariant& target, const MVariant& source);
	bool SetValue(const MString& strName, const MVariant& value);

public:

	MShaderConstantParam* FindConstantParam(const MShaderConstantParam* pParam) { return FindShaderParam(m_vParams, pParam); }
	void AppendConstantParam(MShaderConstantParam* pParam, const uint32_t& eShaderType) { return AppendShaderParam(m_vParams, pParam, eShaderType); }
	std::vector<MShaderConstantParam*> RemoveConstantParam(const uint32_t& eShaderType) { return RemoveShaderParam<MShaderConstantParam>(m_vParams, eShaderType); }

	MShaderTextureParam* FindTextureParam(const MShaderTextureParam* pParam) { return FindShaderParam(m_vTextures, pParam); }
	void AppendTextureParam(MShaderTextureParam* pParam, const uint32_t& eShaderType) { return AppendShaderParam(m_vTextures, pParam, eShaderType); }
	std::vector<MShaderTextureParam*> RemoveTextureParam(const uint32_t& eShaderType) { return RemoveShaderParam<MShaderTextureParam>(m_vTextures, eShaderType); }

	MShaderSampleParam* FindSampleParam(const MShaderSampleParam* pParam) { return FindShaderParam(m_vSamples, pParam); }
	void AppendSampleParam(MShaderSampleParam* pParam, const uint32_t& eShaderType) { return AppendShaderParam(m_vSamples, pParam, eShaderType); }
	std::vector<MShaderSampleParam*> RemoveSampleParam(const uint32_t& eShaderType) { return RemoveShaderParam<MShaderSampleParam>(m_vSamples, eShaderType); }

	void GenerateBuffer(MIDevice* pDevice);
	void DestroyBuffer(MIDevice* pDevice);

	MShaderParamSet* Clone();

public:
	std::vector<MShaderConstantParam*> m_vParams;
	std::vector<MShaderTextureParam*> m_vTextures;
	std::vector<MShaderSampleParam*> m_vSamples;

public:
	uint32_t m_unKey;

#if RENDER_GRAPHICS == MORTY_VULKAN
	VkDescriptorSet m_VkDescriptorSet;
	int m_nDescriptorSetInitMaterialIdx;
	uint32_t m_unLayoutDataIdx;
#endif

protected:

	template <typename ParamType>
	ParamType* FindShaderParam(std::vector<ParamType*>& vVector, const MShaderParam* pParam);

    template <typename ParamType>
	void AppendShaderParam(std::vector<ParamType*>& vVector, ParamType* pParam, const uint32_t& eShaderType);

	template <typename ParamType>
	std::vector<ParamType*> RemoveShaderParam(std::vector<ParamType*>& vVector, const uint32_t& eShaderType);

};

template <typename ParamType>
ParamType* MShaderParamSet::FindShaderParam(std::vector<ParamType*>& vVector, const MShaderParam* pParam)
{
	uint32_t unNum = vVector.size();
	for (uint32_t i = 0; i < unNum; ++i)
	{
		ParamType* param = vVector[i];

		if (pParam->unSet == param->unSet && pParam->unBinding == param->unBinding)
		{
			return vVector[i];
		}
	}

	return nullptr;
}

template <typename ParamType>
void MShaderParamSet::AppendShaderParam(std::vector<ParamType*>& vVector, ParamType* pParam, const uint32_t& eShaderType)
{
	vVector.push_back(pParam);
}

template <typename ParamType>
std::vector<ParamType*> MShaderParamSet::RemoveShaderParam(std::vector<ParamType*>& vVector, const uint32_t& eShaderType)
{
	std::vector<ParamType*> vResult;

	for (auto iter = vVector.begin(); iter != vVector.end();)
	{
		ParamType* param = *iter;

		if (param->eShaderType & eShaderType)
		{
			param->eShaderType = param->eShaderType ^ eShaderType;
		}

		if (0 == param->eShaderType)
		{
			vResult.push_back(*iter);
			iter = vVector.erase(iter);
		}
		else
		{
			++iter;
		}
	}

	return vResult;
}

#endif
