#include "MShaderGroup.h"

#include "Material/MShader.h"
#include "MShaderBuffer.h"
#include "Resource/MShaderResource.h"
#include "Resource/MTextureResource.h"
#include "Resource/MMaterialResource.h"
#include "Engine/MEngine.h"
#include "Render/MIDevice.h"
#include "Utility/MFileHelper.h"

#include "System/MRenderSystem.h"
#include "System/MResourceSystem.h"

#include "Utility/MJson.h"
#include "Utility/MVariant.h"


MORTY_CLASS_IMPLEMENT(MShaderGroup, MTypeClass)

MShaderGroup::MShaderGroup()
	: MTypeClass()
	, m_VertexResource(nullptr)
	, m_PixelResource(nullptr)
	, m_ComputeResource(nullptr)
	, m_pVertexShader(nullptr)
	, m_pPixelShader(nullptr)
	, m_pComputeShader(nullptr)
	, m_nVertexShaderIndex(0)
	, m_nPixelShaderIndex(0)
	, m_nComputeShaderIndex(0)
	, m_ShaderMacro()
{
    m_vShaderSets[MRenderGlobal::SHADER_PARAM_SET_MATERIAL] = MShaderParamSet(MRenderGlobal::SHADER_PARAM_SET_MATERIAL);
    m_vShaderSets[MRenderGlobal::SHADER_PARAM_SET_FRAME] = MShaderParamSet(MRenderGlobal::SHADER_PARAM_SET_FRAME);
    m_vShaderSets[MRenderGlobal::SHADER_PARAM_SET_MESH] = MShaderParamSet(MRenderGlobal::SHADER_PARAM_SET_MESH);
    m_vShaderSets[MRenderGlobal::SHADER_PARAM_SET_SKELETON] = MShaderParamSet(MRenderGlobal::SHADER_PARAM_SET_SKELETON);
}

MShaderGroup::~MShaderGroup()
{

}

bool MShaderGroup::LoadVertexShader(MEngine* pEngine, std::shared_ptr<MResource> pResource)
{
	if (std::shared_ptr<MShaderResource> pShaderResource = MTypeClass::DynamicCast<MShaderResource>(pResource))
	{
		if (MEShaderType::EVertex == pShaderResource->GetShaderType())
		{
			auto LoadFunc = [this, pEngine]() {
				MShaderResource* pShaderResource = m_VertexResource.GetResource()->DynamicCast<MShaderResource>();
				if (nullptr == pShaderResource)
					return false;

				m_nVertexShaderIndex = pShaderResource->FindShaderByMacroParam(m_ShaderMacro);
				m_pVertexShader = pShaderResource->GetShaderByIndex(m_nVertexShaderIndex);
				if (m_pVertexShader && nullptr == m_pVertexShader->GetBuffer())
				{
					MRenderSystem* pRenderSystem = pEngine->FindSystem<MRenderSystem>();
					if (!m_pVertexShader->CompileShader(pRenderSystem->GetDevice()))
					{
						assert(false);
						m_pVertexShader = nullptr;
						return false;
					}
				}
				UnbindShaderBuffer(pEngine, MEShaderParamType::EVertex);
				BindShaderBuffer(m_pVertexShader->GetBuffer(), MEShaderParamType::EVertex);
				return true;
			};



			m_VertexResource.SetResource(pResource);
			m_VertexResource.SetResChangedCallback(LoadFunc);

			LoadFunc();
			return true;
		}
	}

	return false;
}

bool MShaderGroup::LoadPixelShader(MEngine* pEngine, std::shared_ptr<MResource> pResource)
{
	if (std::shared_ptr<MShaderResource> pShaderResource = MTypeClass::DynamicCast<MShaderResource>(pResource))
	{
		if (MEShaderType::EPixel == pShaderResource->GetShaderType())
		{
			auto LoadFunc = [this, pEngine]() {
				MShaderResource* pShaderResource = m_PixelResource.GetResource()->DynamicCast<MShaderResource>();
				if (nullptr == pShaderResource)
					return false;

				m_nPixelShaderIndex = pShaderResource->FindShaderByMacroParam(m_ShaderMacro);
				m_pPixelShader = pShaderResource->GetShaderByIndex(m_nPixelShaderIndex);
				if (m_pPixelShader && nullptr == m_pPixelShader->GetBuffer())
				{
					MRenderSystem* pRenderSystem = pEngine->FindSystem<MRenderSystem>();
					if (!m_pPixelShader->CompileShader(pRenderSystem->GetDevice()))
					{
						m_pPixelShader = nullptr;
						return false;
					}
				}

				UnbindShaderBuffer(pEngine, MEShaderParamType::EPixel);
				BindShaderBuffer(m_pPixelShader->GetBuffer(), MEShaderParamType::EPixel);
				return true;
			};

			m_PixelResource.SetResource(pResource);
			m_PixelResource.SetResChangedCallback(LoadFunc);

			LoadFunc();
			return true;
		}
	}

	return false;
}

bool MShaderGroup::LoadComputeShader(MEngine* pEngine, std::shared_ptr<MResource> pResource)
{
	if (std::shared_ptr<MShaderResource> pShaderResource = MTypeClass::DynamicCast<MShaderResource>(pResource))
	{
		if (MEShaderType::ECompute == pShaderResource->GetShaderType())
		{
			auto LoadFunc = [this, pEngine]() {
				MShaderResource* pShaderResource = m_ComputeResource.GetResource()->DynamicCast<MShaderResource>();
				if (nullptr == pShaderResource)
					return false;

				m_nComputeShaderIndex = pShaderResource->FindShaderByMacroParam(m_ShaderMacro);
				m_pComputeShader = pShaderResource->GetShaderByIndex(m_nComputeShaderIndex);
				if (m_pComputeShader && nullptr == m_pComputeShader->GetBuffer())
				{
					MRenderSystem* pRenderSystem = pEngine->FindSystem<MRenderSystem>();
					if (!m_pComputeShader->CompileShader(pRenderSystem->GetDevice()))
					{
						m_pComputeShader = nullptr;
						return false;
					}
				}

				UnbindShaderBuffer(pEngine, MEShaderParamType::ECompute);
				BindShaderBuffer(m_pComputeShader->GetBuffer(), MEShaderParamType::ECompute);
				return true;
			};

			m_ComputeResource.SetResource(pResource);
			m_ComputeResource.SetResChangedCallback(LoadFunc);

			LoadFunc();
			return true;
		}
	}

	return false;
}

void MShaderGroup::BindShaderBuffer(MShaderBuffer* pBuffer, const MEShaderParamType& eType)
{
	for (uint32_t i = 0; i < MRenderGlobal::SHADER_PARAM_SET_NUM; ++i)
	{
		MShaderParamSet& bufferSet = pBuffer->m_vShaderSets[i];
		MShaderParamSet& selfSet = m_vShaderSets[i];

		for (MShaderConstantParam* pBufferParam : bufferSet.m_vParams)
		{
			if (MShaderConstantParam* pSelfParam = selfSet.FindConstantParam(pBufferParam))
			{
				pSelfParam->eShaderType |= eType;
				MVariant temp;
				temp.Move(pSelfParam->var);
				pSelfParam->var = pBufferParam->var;
				pSelfParam->var.MergeFrom(temp);
				pSelfParam->SetDirty();
			}
			else
			{
				MShaderConstantParam* pParam = new MShaderConstantParam(*pBufferParam, 0);
				pParam->eShaderType = eType;
				selfSet.AppendConstantParam(pParam, eType);
			}
		}

		for (MShaderTextureParam* pBufferParam : bufferSet.m_vTextures)
		{
			if (MShaderTextureParam* pSelfParam = selfSet.FindTextureParam(pBufferParam))
			{
				pSelfParam->eShaderType |= eType;
			}
			else
			{
				MShaderTextureParam* pParam = new MTextureResourceParam(*pBufferParam);
				pParam->eShaderType = eType;
				selfSet.AppendTextureParam(pParam, eType);
			}
		}

		for (MShaderSampleParam* pBufferParam : bufferSet.m_vSamples)
		{
			if (MShaderSampleParam* pSelfParam = selfSet.FindSampleParam(pBufferParam))
			{
				pSelfParam->eShaderType |= eType;
			}
			else
			{
				MShaderSampleParam* pParam = new MShaderSampleParam(*pBufferParam);
				pParam->eShaderType = eType;
				selfSet.AppendSampleParam(pParam, eType);
			}
		}

		for (MShaderStorageParam* pBufferParam : bufferSet.m_vStorages)
		{
			if (MShaderStorageParam* pSelfParam = selfSet.FindStorageParam(pBufferParam))
			{
				pSelfParam->eShaderType |= eType;
			}
			else
			{
				MShaderStorageParam* pParam = new MShaderStorageParam(*pBufferParam);
				pParam->eShaderType = eType;
				selfSet.AppendStorageParam(pParam, eType);
			}
		}
	}
}

void MShaderGroup::UnbindShaderBuffer(MEngine* pEngine, const MEShaderParamType& eType)
{
	MRenderSystem* pRenderSystem = pEngine->FindSystem<MRenderSystem>();
	for (uint32_t i = 0; i < MRenderGlobal::SHADER_PARAM_SET_NUM; ++i)
	{
		MShaderParamSet& selfSet = m_vShaderSets[i];
		std::vector<MShaderConstantParam*> vConstantParams = selfSet.RemoveConstantParam(eType);
		std::vector<MShaderTextureParam*> vTextureParams = selfSet.RemoveTextureParam(eType);
		std::vector<MShaderSampleParam*> vSampleParams = selfSet.RemoveSampleParam(eType);
		std::vector<MShaderStorageParam*> vStorageParams = selfSet.RemoveStorageParam(eType);

		for (MShaderConstantParam* pParam : vConstantParams)
		{
			pRenderSystem->GetDevice()->DestroyShaderParamBuffer(pParam);
			delete pParam;
		}

		for (MShaderTextureParam* pParam : vTextureParams)
		{
			delete pParam;
		}

		for (MShaderSampleParam* pParam : vSampleParams)
		{
			delete pParam;
		}

		for (MShaderStorageParam* pParam : vStorageParams)
		{
			delete pParam;
		}
	}
}

void MShaderGroup::ClearShader(MEngine* pEngine)
{
	MRenderSystem* pRenderSystem = pEngine->FindSystem<MRenderSystem>();
	for (uint32_t i = 0; i < MRenderGlobal::SHADER_PARAM_SET_NUM; ++i)
	{
		m_vShaderSets[i].DestroyBuffer(pRenderSystem->GetDevice());
		m_vShaderSets[i] = MShaderParamSet(i);
	}

	m_VertexResource.SetResource(nullptr);
	m_PixelResource.SetResource(nullptr);;
	m_ComputeResource.SetResource(nullptr);;

	m_pVertexShader = nullptr;
	m_pPixelShader = nullptr;
	m_pComputeShader = nullptr;

	m_nVertexShaderIndex = 0;
	m_nPixelShaderIndex = 0;
	m_nComputeShaderIndex = 0;
}

void MShaderGroup::CopyShaderParams(MEngine* pEngine, MShaderParamSet& target, const MShaderParamSet& source)
{
	MRenderSystem* pRenderSystem = pEngine->FindSystem<MRenderSystem>();

	target.DestroyBuffer(pRenderSystem->GetDevice());

	target.m_vParams.resize(source.m_vParams.size());
	for (uint32_t i = 0; i < source.m_vParams.size(); ++i)
	{
		target.m_vParams[i] = new MShaderConstantParam(*source.m_vParams[i], 0);
	}

	target.m_vTextures.resize(source.m_vTextures.size());
	for (uint32_t i = 0; i < source.m_vTextures.size(); ++i)
	{
		MTextureResourceParam* pSource = static_cast<MTextureResourceParam*>(source.m_vTextures[i]);
		MTextureResourceParam* pParam = new MTextureResourceParam(*pSource);

		pParam->SetTexture(pSource->GetTextureResource());

		target.m_vTextures[i] = pParam;
	}

	target.m_vSamples.resize(source.m_vSamples.size());
	for (uint32_t i = 0; i < source.m_vSamples.size(); ++i)
	{
		target.m_vSamples[i] = new MShaderSampleParam(*source.m_vSamples[i]);
	}

	target.m_vStorages.resize(source.m_vStorages.size());
	for (uint32_t i = 0; i < source.m_vStorages.size(); ++i)
	{
		target.m_vStorages[i] = new MShaderStorageParam(*source.m_vStorages[i]);
	}
}

MTextureResourceParam::MTextureResourceParam()
	: MShaderTextureParam()
	, m_TextureRef()
{

}

MTextureResourceParam::MTextureResourceParam(const MShaderTextureParam& param)
	: MShaderTextureParam(param)
	, m_TextureRef()
{

}

void MTextureResourceParam::SetTexture(MTexture* pTexture)
{
	m_TextureRef.SetResource(nullptr);

	MShaderTextureParam::SetTexture(pTexture);
}

void MTextureResourceParam::SetTexture(const std::shared_ptr<MTextureResource>& pTextureResource)
{
	static auto onResourceChangedFunction = [this]() {
		SetDirty();
		return true;
	};

	m_TextureRef.SetResource(pTextureResource);
	m_TextureRef.SetResChangedCallback(onResourceChangedFunction);
}

MTexture* MTextureResourceParam::GetTexture()
{
	if (auto&& pTextureResource = m_TextureRef.GetResource<MTextureResource>())
	{
		return pTextureResource->GetTextureTemplate();
	}

	return MShaderTextureParam::GetTexture();
}
