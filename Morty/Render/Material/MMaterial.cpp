#include "Material/MMaterial.h"
#include "Material/MShader.h"
#include "MShaderBuffer.h"
#include "Resource/MShaderResource.h"
#include "Resource/MTextureResource.h"
#include "Resource/MMaterialResource.h"
#include "Engine/MEngine.h"
#include "Flatbuffer/MMaterial_generated.h"
#include "Render/MIDevice.h"
#include "Utility/MFileHelper.h"

#include "System/MRenderSystem.h"
#include "System/MResourceSystem.h"

#include "Variant/MVariant.h"


MORTY_CLASS_IMPLEMENT(MMaterial, MResource)

MMaterial::MMaterial()
	: MResource()
	, m_pShaderProgram(MShaderProgram::MakeShared(MShaderProgram::EUsage::EGraphics))
	, m_eRasterizerType(MERasterizerType::ECullBack)
	, m_eMaterialType(MEMaterialType::EDefault)
	, m_unMaterialID(MGlobal::M_INVALID_INDEX)
{
}

MMaterial::~MMaterial()
{
}

std::vector<std::shared_ptr<MShaderConstantParam>>& MMaterial::GetShaderParams()
{
	return m_pShaderProgram->GetShaderParamSets()[MRenderGlobal::SHADER_PARAM_SET_MATERIAL]->m_vParams;
}

std::vector<std::shared_ptr<MShaderSampleParam>>& MMaterial::GetSampleParams()
{
	return m_pShaderProgram->GetShaderParamSets()[MRenderGlobal::SHADER_PARAM_SET_MATERIAL]->m_vSamples;
}

std::vector<std::shared_ptr<MShaderTextureParam>>& MMaterial::GetTextureParams()
{
	return m_pShaderProgram->GetShaderParamSets()[MRenderGlobal::SHADER_PARAM_SET_MATERIAL]->m_vTextures;
}

void MMaterial::SetTexture(const MString& strName, std::shared_ptr<MResource> pResource)
{
	for (int i = 0; i < GetMaterialParamSet()->m_vTextures.size(); ++i)
	{
		const std::shared_ptr<MTextureResourceParam>& pParam = std::dynamic_pointer_cast<MTextureResourceParam>(GetMaterialParamSet()->m_vTextures[i]);
		if (strName == pParam->strName)
		{
			if (std::shared_ptr<MTextureResource> pTexResource = MTypeClass::DynamicCast<MTextureResource>(pResource))
			{
				if (std::shared_ptr<MTexture> pTexture = pTexResource->GetTextureTemplate())
				{
					if (pTexture->GetTextureType() != pParam->eType)
					{
						break;
					}
				}
				
				pParam->SetTexture(pTexResource);
			}

			break;
		}
	} 
}

std::shared_ptr<MShaderConstantParam> MMaterial::FindShaderParam(const MString& strName)
{
	for (const std::shared_ptr<MShaderConstantParam>& pParam : GetMaterialParamSet()->m_vParams)
	{
		if (pParam->strName == strName)
			return pParam;
	}

	return nullptr;
}

std::shared_ptr<MShaderSampleParam> MMaterial::FindSample(const MString& strName)
{
	for (const std::shared_ptr<MShaderSampleParam>& pParam : GetMaterialParamSet()->m_vSamples)
	{
		if (pParam->strName == strName)
			return pParam;
	}
}

std::shared_ptr<MShaderTextureParam> MMaterial::FindTexture(const MString& strName)
{
	for (const std::shared_ptr<MShaderTextureParam>& pParam : GetMaterialParamSet()->m_vTextures)
	{
		if (pParam->strName == strName)
			return pParam;
	}
}

void MMaterial::CopyFrom(std::shared_ptr<const MResource> pResource)
{
	Unload();

	std::shared_ptr<const  MMaterialResource> pMaterial = MTypeClass::DynamicCast<const MMaterialResource>(pResource);
	if (nullptr == pMaterial)
		return;

	//Material
	m_eRasterizerType = pMaterial->m_eRasterizerType;
	m_eMaterialType = pMaterial->m_eMaterialType;

	*m_pShaderProgram = *pMaterial->m_pShaderProgram;

	for (uint32_t i = 0; i < MRenderGlobal::SHADER_PARAM_SET_NUM; ++i)
	{
		MShaderProgram::CopyShaderParams(GetEngine(), GetShaderParamSets()[i], pMaterial->GetShaderParamSets()[i]);
	}
}

flatbuffers::Offset<void> MMaterial::Serialize(flatbuffers::FlatBufferBuilder& fbb) const
{
	const auto fbVertexShader = MResourceRef(GetVertexShaderResource()).Serialize(fbb);
	const auto fbPixelShader = MResourceRef(GetPixelShaderResource()).Serialize(fbb);
	const auto fbMacro = GetShaderMacro().Serialize(fbb);

	std::vector<flatbuffers::Offset<mfbs::MMaterialTexture>> vTextures;
	for (const std::shared_ptr<MShaderTextureParam>& pParam : GetMaterialParamSet()->m_vTextures)
	{
		const std::shared_ptr<MTextureResourceParam> pResourceParam = std::dynamic_pointer_cast<MTextureResourceParam>(pParam);
		const auto fbName = fbb.CreateString(pResourceParam->strName);
		const auto fbTexture = MResourceRef(pResourceParam->GetTextureResource()).Serialize(fbb);

		mfbs::MMaterialTextureBuilder builder(fbb);
		builder.add_name(fbName);
		builder.add_texture(fbTexture.o);
		vTextures.push_back(builder.Finish());
	}

	std::vector<flatbuffers::Offset<mfbs::MMaterialProperty>> vProperty;
	for (std::shared_ptr<MShaderConstantParam> pParam : GetMaterialParamSet()->m_vParams)
	{
		const auto fbName = fbb.CreateString(pParam->strName);
		const auto fbProperty = pParam->var.Serialize(fbb);

		mfbs::MMaterialPropertyBuilder builder(fbb);
		builder.add_name(fbName);
		builder.add_property(fbProperty.o);
		vProperty.push_back(builder.Finish());
	}

	const auto fbTextures = fbb.CreateVector(vTextures);
	const auto fbProperty = fbb.CreateVector(vProperty);

	mfbs::MMaterialBuilder builder(fbb);

	builder.add_vertex_resource(fbVertexShader.o);
	builder.add_pixel_resource(fbPixelShader.o);
	builder.add_material_macro(fbMacro.o);
	builder.add_material_type(static_cast<mfbs::MEMaterialType>(m_eMaterialType));
	builder.add_rasterizer_type(static_cast<mfbs::MERasterizerType>(m_eRasterizerType));

	builder.add_material_textures(fbTextures);
	builder.add_material_property(fbProperty);

	return builder.Finish().Union();
}

void MMaterial::Deserialize(const void* pBufferPointer)
{
	const mfbs::MMaterial* fbData = reinterpret_cast<const mfbs::MMaterial*>(pBufferPointer);

	m_pShaderProgram->ClearShader(GetEngine());
	m_pShaderProgram = MShaderProgram::MakeShared(MShaderProgram::EUsage::EGraphics);

	m_eMaterialType = static_cast<MEMaterialType>(fbData->material_type());
	m_eRasterizerType = static_cast<MERasterizerType>(fbData->rasterizer_type());

	GetShaderMacro().Deserialize(fbData->material_macro());
	MResourceRef vertexShader, pixelShader;
	vertexShader.Deserialize(GetEngine(), fbData->vertex_resource());
	pixelShader.Deserialize(GetEngine(), fbData->pixel_resource());

	MORTY_ASSERT(LoadVertexShader(vertexShader.GetResource()));
	MORTY_ASSERT(LoadPixelShader(pixelShader.GetResource()));



	if (fbData->material_property())
	{
		const size_t nPropertyNum = fbData->material_property()->size();
		for (size_t nIdx = 0; nIdx < nPropertyNum; ++nIdx)
		{
			const auto fbProperty = fbData->material_property()->Get(nIdx);
			MString strPropertyName = fbProperty->name()->c_str();

			if (std::shared_ptr<MShaderConstantParam> pConstantParam = GetMaterialParamSet()->FindConstantParam(strPropertyName))
			{
				pConstantParam->var.Deserialize(fbProperty->property());
				pConstantParam->SetDirty();
			}
		}
	}

	if (fbData->material_textures())
	{
		const size_t nTextureNum = fbData->material_textures()->size();
		for (size_t nIdx = 0; nIdx < nTextureNum; ++nIdx)
		{
			const auto fbTexture = fbData->material_textures()->Get(nIdx);

			MResourceRef resource;
			resource.Deserialize(GetEngine(), fbTexture->texture());

			if( auto pTextureResource = resource.GetResource<MTextureResource>())
			{
				SetTexture(fbTexture->name()->c_str(), pTextureResource);
			}
		}
	}

}

bool MMaterial::SaveTo(const MString& strResourcePath)
{
	flatbuffers::FlatBufferBuilder fbb;
	auto fbData = Serialize(fbb);
	fbb.Finish(fbData);

	std::vector<MByte> data(fbb.GetSize());
	memcpy(data.data(), (MByte*)fbb.GetBufferPointer(), fbb.GetSize() * sizeof(MByte));

	return MFileHelper::WriteData(strResourcePath, data);
}

bool MMaterial::Load(const MString& strResourcePath)
{
	std::vector<MByte> data;
	MFileHelper::ReadData(strResourcePath, data);

	flatbuffers::FlatBufferBuilder fbb;
	fbb.PushBytes((const uint8_t*)data.data(), data.size());

	const mfbs::MMaterial* fbMeshResource = mfbs::GetMMaterial(fbb.GetCurrentBufferPointer());
	Deserialize(fbMeshResource);

	return true;
}

void MMaterial::SetRasterizerType(const MERasterizerType& eType)
{
	if (m_eRasterizerType == eType)
		return;

	m_eRasterizerType = eType;

	MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();

	std::shared_ptr<MMaterial> self = MTypeClass::DynamicCast<MMaterial>(GetShared());

	if (pRenderSystem->GetDevice()->UnRegisterMaterial(self))
	{
		pRenderSystem->GetDevice()->RegisterMaterial(self);
	}
}

void MMaterial::SetMaterialType(const MEMaterialType& eType)
{
	if (m_eMaterialType == eType)
		return;

	m_eMaterialType = eType;

	switch (m_eMaterialType)
	{
	case MEMaterialType::EDepthPeel:
	{
		GetShaderMacro().SetInnerMacro("MEN_TRANSPARENT", "1");
		LoadVertexShader(GetVertexShaderResource());
		LoadPixelShader(GetPixelShaderResource());
		break;
	}

	default:
		GetShaderMacro().SetInnerMacro("MEN_TRANSPARENT", "0");
		LoadVertexShader(GetVertexShaderResource());
		LoadPixelShader(GetPixelShaderResource());
		break;
	}

	MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();

	std::shared_ptr<MMaterial> self = MTypeClass::DynamicCast<MMaterial>(GetShared());

	if (pRenderSystem->GetDevice()->UnRegisterMaterial(self))
	{
		pRenderSystem->GetDevice()->RegisterMaterial(self);
	}
}

bool MMaterial::LoadVertexShader(std::shared_ptr<MResource> pResource)
{
	bool bResult = m_pShaderProgram->LoadVertexShader(GetEngine(), pResource);

	OnReload();

	return bResult;
}

bool MMaterial::LoadVertexShader(const MString& strResource)
{
	MResourceSystem* pResourceSystem = GetEngine()->FindSystem<MResourceSystem>();
	if (std::shared_ptr<MResource> pResource = pResourceSystem->LoadResource(strResource))
		return LoadVertexShader(pResource);

	return false;
}

bool MMaterial::LoadPixelShader(std::shared_ptr<MResource> pResource)
{
	bool bResult = m_pShaderProgram->LoadPixelShader(GetEngine(), pResource);

	OnReload();

	return bResult;
}

bool MMaterial::LoadPixelShader(const MString& strResource)
{
	MResourceSystem* pResourceSystem = GetEngine()->FindSystem<MResourceSystem>();
	if (std::shared_ptr<MResource> pResource = pResourceSystem->LoadResource(strResource))
		return LoadPixelShader(pResource);

	return false;
}

bool MMaterial::LoadComputeShader(std::shared_ptr<MResource> pResource)
{
	bool bResult = m_pShaderProgram->LoadComputeShader(GetEngine(), pResource);

	OnReload();

	return bResult;
}

bool MMaterial::LoadComputeShader(const MString& strResource)
{
	MResourceSystem* pResourceSystem = GetEngine()->FindSystem<MResourceSystem>();
	if (std::shared_ptr<MResource> pResource = pResourceSystem->LoadResource(strResource))
		return LoadComputeShader(pResource);

	return false;
}

void MMaterial::OnCreated()
{
	Super::OnCreated();

	std::shared_ptr<MMaterial> self = MTypeClass::DynamicCast<MMaterial>(GetShared());

	MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();
	pRenderSystem->GetDevice()->RegisterMaterial(self);
}

void MMaterial::OnDelete()
{
	MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();
	if (pRenderSystem->GetDevice())
	{
		std::shared_ptr<MMaterial> self = MTypeClass::DynamicCast<MMaterial>(GetShared());
		pRenderSystem->GetDevice()->UnRegisterMaterial(self);
	}

	m_pShaderProgram->ClearShader(GetEngine());
		
	MResource::OnDelete();
}

void MMaterial::Unload()
{
	m_pShaderProgram->ClearShader(GetEngine());
}

const MString MaterialKey::Albedo = "u_mat_texAlbedo";
const MString MaterialKey::Normal = "u_texNormal";
const MString MaterialKey::Metallic = "u_mat_texMetallic";
const MString MaterialKey::Roughness = "u_mat_texRoughness";
const MString MaterialKey::AmbientOcc = "u_mat_texAmbientOcc";
const MString MaterialKey::Height = "u_mat_texHeight";
