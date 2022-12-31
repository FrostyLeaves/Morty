#include "MShaderProgram.h"

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


MORTY_CLASS_IMPLEMENT(MShaderProgram, MTypeClass)

MShaderProgram::MShaderProgram(EUsage usage)
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
	, m_eUsage(usage)
{
}

void MShaderProgram::InitializeShaderPropertyBlock()
{
	m_vShaderSets[MRenderGlobal::SHADER_PARAM_SET_MATERIAL] = MShaderPropertyBlock::MakeShared(GetShared(), MRenderGlobal::SHADER_PARAM_SET_MATERIAL);
	m_vShaderSets[MRenderGlobal::SHADER_PARAM_SET_FRAME] = MShaderPropertyBlock::MakeShared(GetShared(), MRenderGlobal::SHADER_PARAM_SET_FRAME);
	m_vShaderSets[MRenderGlobal::SHADER_PARAM_SET_MESH] = MShaderPropertyBlock::MakeShared(GetShared(), MRenderGlobal::SHADER_PARAM_SET_MESH);
	m_vShaderSets[MRenderGlobal::SHADER_PARAM_SET_SKELETON] = MShaderPropertyBlock::MakeShared(GetShared(), MRenderGlobal::SHADER_PARAM_SET_SKELETON);
}

std::shared_ptr<MShaderProgram> MShaderProgram::MakeShared(EUsage usage)
{
	std::shared_ptr<MShaderProgram> pResult = std::make_shared<MShaderProgram>(usage);
	pResult->m_pSelfPointer = pResult;
	pResult->InitializeShaderPropertyBlock();
	return pResult;
}


MShaderProgram::~MShaderProgram()
{

}

bool MShaderProgram::LoadVertexShader(MEngine* pEngine, std::shared_ptr<MResource> pResource)
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
						MORTY_ASSERT(false);
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

bool MShaderProgram::LoadPixelShader(MEngine* pEngine, std::shared_ptr<MResource> pResource)
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

bool MShaderProgram::LoadComputeShader(MEngine* pEngine, std::shared_ptr<MResource> pResource)
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

void MShaderProgram::BindShaderBuffer(MShaderBuffer* pBuffer, const MEShaderParamType& eType)
{
	for (uint32_t i = 0; i < MRenderGlobal::SHADER_PARAM_SET_NUM; ++i)
	{
		std::shared_ptr<MShaderPropertyBlock>& pPropertyTemplate = pBuffer->m_vShaderSets[i];
		std::shared_ptr<MShaderPropertyBlock>& pProgramProperty = m_vShaderSets[i];

		for (const std::shared_ptr<MShaderConstantParam>& pBufferParam : pPropertyTemplate->m_vParams)
		{
			if (std::shared_ptr<MShaderConstantParam> pSelfParam = pProgramProperty->FindConstantParam(pBufferParam))
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
				std::shared_ptr<MShaderConstantParam> pParam = std::make_shared<MShaderConstantParam>(*pBufferParam, 0);
				pParam->eShaderType = eType;
				pProgramProperty->AppendConstantParam(pParam, eType);
			}
		}

		for (const std::shared_ptr<MShaderTextureParam>& pBufferParam : pPropertyTemplate->m_vTextures)
		{
			if (std::shared_ptr<MShaderTextureParam> pSelfParam = pProgramProperty->FindTextureParam(pBufferParam))
			{
				pSelfParam->eShaderType |= eType;
			}
			else
			{
				std::shared_ptr<MShaderTextureParam> pParam = std::make_shared<MTextureResourceParam>(*pBufferParam);
				pParam->eShaderType = eType;
				pProgramProperty->AppendTextureParam(pParam, eType);
			}
		}

		for (const std::shared_ptr<MShaderSampleParam>& pBufferParam : pPropertyTemplate->m_vSamples)
		{
			if (std::shared_ptr<MShaderSampleParam> pSelfParam = pProgramProperty->FindSampleParam(pBufferParam))
			{
				pSelfParam->eShaderType |= eType;
			}
			else
			{
				std::shared_ptr<MShaderSampleParam> pParam = std::make_shared<MShaderSampleParam>(*pBufferParam);
				pParam->eShaderType = eType;
				pProgramProperty->AppendSampleParam(pParam, eType);
			}
		}

		for (const std::shared_ptr<MShaderStorageParam>& pBufferParam : pPropertyTemplate->m_vStorages)
		{
			if (std::shared_ptr<MShaderStorageParam> pSelfParam = pProgramProperty->FindStorageParam(pBufferParam))
			{
				pSelfParam->eShaderType |= eType;
			}
			else
			{
				std::shared_ptr<MShaderStorageParam> pParam = std::make_shared<MShaderStorageParam>(*pBufferParam);
				pParam->eShaderType = eType;
				pProgramProperty->AppendStorageParam(pParam, eType);
			}
		}
	}
}

void MShaderProgram::UnbindShaderBuffer(MEngine* pEngine, const MEShaderParamType& eType)
{
	MRenderSystem* pRenderSystem = pEngine->FindSystem<MRenderSystem>();
	for (uint32_t i = 0; i < MRenderGlobal::SHADER_PARAM_SET_NUM; ++i)
	{
		std::shared_ptr<MShaderPropertyBlock>& pProgramProperty = m_vShaderSets[i];
		std::vector<std::shared_ptr<MShaderConstantParam>>&& vConstantParams = pProgramProperty->RemoveConstantParam(eType);
		std::vector<std::shared_ptr<MShaderTextureParam>>&& vTextureParams = pProgramProperty->RemoveTextureParam(eType);
		std::vector<std::shared_ptr<MShaderSampleParam>>&& vSampleParams = pProgramProperty->RemoveSampleParam(eType);
		std::vector<std::shared_ptr<MShaderStorageParam>>&& vStorageParams = pProgramProperty->RemoveStorageParam(eType);

		for (std::shared_ptr<MShaderConstantParam>& pParam : vConstantParams)
		{
			pRenderSystem->GetDevice()->DestroyShaderParamBuffer(pParam);
		}
	}
}

void MShaderProgram::ClearShader(MEngine* pEngine)
{
	MRenderSystem* pRenderSystem = pEngine->FindSystem<MRenderSystem>();
	for (uint32_t i = 0; i < MRenderGlobal::SHADER_PARAM_SET_NUM; ++i)
	{
		m_vShaderSets[i]->DestroyBuffer(pRenderSystem->GetDevice());
		m_vShaderSets[i] = nullptr;
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

void MShaderProgram::CopyShaderParams(MEngine* pEngine, const std::shared_ptr<MShaderPropertyBlock>& target, const std::shared_ptr<const MShaderPropertyBlock>& source)
{
	MRenderSystem* pRenderSystem = pEngine->FindSystem<MRenderSystem>();

	target->DestroyBuffer(pRenderSystem->GetDevice());

	target->m_vParams.resize(source->m_vParams.size());
	for (uint32_t i = 0; i < source->m_vParams.size(); ++i)
	{
		target->m_vParams[i] = std::make_shared<MShaderConstantParam>(*source->m_vParams[i], 0);
	}

	target->m_vTextures.resize(source->m_vTextures.size());
	for (uint32_t i = 0; i < source->m_vTextures.size(); ++i)
	{
		std::shared_ptr<MTextureResourceParam> pSource = std::dynamic_pointer_cast<MTextureResourceParam>(source->m_vTextures[i]);
		std::shared_ptr<MTextureResourceParam> pParam = std::make_shared<MTextureResourceParam>(*pSource);

		pParam->SetTexture(pSource->GetTextureResource());

		target->m_vTextures[i] = pParam;
	}

	target->m_vSamples.resize(source->m_vSamples.size());
	for (uint32_t i = 0; i < source->m_vSamples.size(); ++i)
	{
		target->m_vSamples[i] = std::make_shared<MShaderSampleParam>(*source->m_vSamples[i]);
	}

	target->m_vStorages.resize(source->m_vStorages.size());
	for (uint32_t i = 0; i < source->m_vStorages.size(); ++i)
	{
		target->m_vStorages[i] = std::make_shared<MShaderStorageParam>(*source->m_vStorages[i]);
	}
}

std::shared_ptr<MShaderPropertyBlock> MShaderProgram::AllocShaderParamSet(size_t nSetIdx)
{
	std::shared_ptr<MShaderPropertyBlock> pShaderParamSet = m_vShaderSets[nSetIdx]->Clone();
	m_tShaderParamSetInstance.insert(pShaderParamSet);

	return pShaderParamSet;
}

void MShaderProgram::ReleaseShaderParamSet(const std::shared_ptr<MShaderPropertyBlock>& pShaderParamSet)
{
	m_tShaderParamSetInstance.erase(pShaderParamSet);
}

void MShaderProgram::GenerateProgram(MIDevice* pDevice)
{
	pDevice->GenerateShaderProgram(this);
}

void MShaderProgram::DestroyProgram(MIDevice* pDevice)
{
	pDevice->DestroyShaderProgram(this);
}

std::shared_ptr<MShaderProgram> MShaderProgram::GetShared() const
{
	return m_pSelfPointer.lock();
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
