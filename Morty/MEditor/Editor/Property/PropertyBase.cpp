#include "PropertyBase.h"

#include "MIRenderer.h"
#include "MMaterial.h"
#include "MResource.h"
#include "Texture/MTextureResource.h"
#include "Material/MMaterialResource.h"
#include "MResourceManager.h"

#include "imgui.h"
#include "imgui_stdlib.h"
#include "ImGuiFileDialog.h"

unsigned int PropertyBase::m_unItemIDPool = 0;
std::map<MString, unsigned int> PropertyBase::m_tItemID = std::map<MString, unsigned int>();

static const char* vTitleList[] = {
		"Default",
		"Model",
		"Shader",
		"Material",
		"Texture",
		"Node",
		"Mesh",
		"Skeleton",
		"SkelAnim",
};

static const char* vFilterList[] = {
	"\0\0",
	".model\0\0",
	".mvs\0.mps\0\0",
	".matl\0\0",
	".png\0.jpg\0.tga\0\0",
	".node\0\0",
	".mesh\0\0",
	".mske\0\0",
	".mseq\0\0"
};

bool PropertyBase::ShowNodeBegin(const MString& strNodeName)
{
	if (ShowNodeBeginWithEx(strNodeName))
	{
		ImGui::NextColumn();
		return true;
	}

	return false;
}

bool PropertyBase::ShowNodeBeginWithEx(const MString& strNodeName)
{
	ImGui::PushID(GetID(strNodeName));
	ImGui::AlignTextToFramePadding();
	if (bool node_open = ImGui::TreeNodeEx("Object", ImGuiTreeNodeFlags_DefaultOpen, "%s", strNodeName.c_str()))
	{
		ImGui::NextColumn();
		ImGui::AlignTextToFramePadding();

		return true;
	}

	ImGui::NextColumn();
	ImGui::NextColumn();
	ImGui::PopID();
	return false;
}

void PropertyBase::ShowNodeExBegin(const MString& strExID)
{
	ImGui::PushID(GetID(strExID));
}

void PropertyBase::ShowNodeExEnd()
{
	ImGui::PopID();
	ImGui::NextColumn();
}

void PropertyBase::ShowNodeEnd()
{
	ImGui::TreePop();
	ImGui::PopID();
}

void PropertyBase::ShowValueBegin(const MString& strValueName)
{
	ImGui::PushID(GetID(strValueName));
	ImGui::AlignTextToFramePadding();
	ImGui::TreeNodeEx(strValueName.c_str(), ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Bullet);
	ImGui::NextColumn();
	ImGui::SetNextItemWidth(-1);
}

void PropertyBase::ShowValueEnd()
{
	ImGui::NextColumn();
	ImGui::PopID();
}

bool PropertyBase::Editbool(bool& value)
{
	return ImGui::Checkbox("", &value);
}

bool PropertyBase::Editbool(int& value)
{
	bool bBool = value > 0;
	if (ImGui::Checkbox("", &bBool))
	{
		value = bBool ? 1 : 0;
		return true;
	}

	return false;
}

bool PropertyBase::Editfloat(float& value, const float& fSpeed, const float& fMin, const float& fMax)
{
	if (value == -0.0f) value = 0.0f;
	return ImGui::DragFloat("", &value, fSpeed, fMin, fMax);
}

bool PropertyBase::EditVector2(Vector2& value, const float& fSpeed /*= 1.0f*/, const float& fMin /*= 0.0f*/, const float& fMax /*= 0.0f*/)
{
	if (value.x == -0.0f) value.x = 0.0f;
	if (value.y == -0.0f) value.y = 0.0f;
	return ImGui::DragFloat2("", value.m, fSpeed, fMin, fMax);
}

bool PropertyBase::EditVector3(Vector3& value, const float& fSpeed, const float& fMin, const float& fMax)
{
	return EditVector3(value.m, fSpeed, fMin, fMax);
}

bool PropertyBase::EditVector3(float* pValue, const float& fSpeed /*= 1.0f*/, const float& fMin /*= 0.0f*/, const float& fMax /*= 0.0f*/)
{
	if (pValue[0] == -0.0f) pValue[0] = 0.0f;
	if (pValue[1] == -0.0f) pValue[1] = 0.0f;
	if (pValue[2] == -0.0f) pValue[2] = 0.0f;
	return ImGui::DragFloat3("", pValue, fSpeed, fMin, fMax);
}

bool PropertyBase::EditMTransform(MTransform& trans)
{
	bool bModify = false;
	ShowValueBegin("Position");
	Vector3 position = trans.GetPosition();
	if (EditVector3(position))
	{
		trans.SetPosition(position);
		bModify = true;
	}
	ShowValueEnd();

	ShowValueBegin("Scale");
	Vector3 scale = trans.GetScale();
	if (EditVector3(scale))
	{
		trans.SetScale(scale);
		bModify = true;
	}
	ShowValueEnd();

	ShowValueBegin("Rotate");
	Vector3& rotate = GetTempValue<Vector3>("Rotate", trans.GetRotation().GetEulerAngle());

	if (EditVector3(rotate, 1.0f, -360.0f, 360.0f))
	{
		Quaternion quat;
		quat.SetEulerAngle(rotate);
		quat.Normalize();
		trans.SetRotation(quat);

		bModify = true;
	}
	ShowValueEnd();

	return bModify;
}

bool PropertyBase::EditEnum(const std::vector<MString>& select, int& index)
{
	if (index >= select.size())
		return false;

	unsigned int unNewIndex = index;
	if (ImGui::BeginCombo("", select[index].c_str())) {

		for (int i = 0; i < select.size(); ++i)
		{
			if (ImGui::Selectable(select[i].c_str(), (index == i)))\
			{
				unNewIndex = i;
			}
		}

		ImGui::EndCombo();
	}

	if (unNewIndex != index)
	{
		index = unNewIndex;
		return true;
	}

	return false;
}

bool PropertyBase::EditMVariant(const MString& strVariantName, MVariant& value)
{
	bool bModified = false;

	switch (value.GetType())
	{
	case MVariant::EBool:
		ShowValueBegin(strVariantName);
		bModified |= Editbool(*value.GetBool());
		ShowValueEnd();
		break;

	case MVariant::EFloat:
	case MVariant::EInt:
		ShowValueBegin(strVariantName);
		bModified |= Editfloat(*value.CastFloatUnsafe());
		ShowValueEnd();
		break;

	case MVariant::EVector3:
		ShowValueBegin(strVariantName);
		bModified |= EditVector3(value.CastFloatUnsafe());
		ShowValueEnd();
		break;

	case MVariant::EArray:
	case MVariant::EStruct:
	if(ShowNodeBegin(strVariantName))
	{
		MContainer* pStruct = value.GetContainer();
		unsigned int unCount = pStruct->GetMemberCount();
		for (unsigned int i = 0; i < unCount; ++i)
		{
			if (MContainer::MStructMember* pMember = pStruct->GetMember(i))
			{
				bModified |= EditMVariant(pMember->strName.empty() ? MStringHelper::ToString(i) : pMember->strName, pMember->var);
			}
		}

		ShowNodeEnd();
		break;
	}

	case MVariant::ENone:
	default:
		break;
	}

	return bModified;
}

bool PropertyBase::EditMMaterial(MMaterial* pMaterial)
{
	bool bModified = false;

	
	if (MMaterialResource* pResource = pMaterial)
	{
		{
			ShowValueBegin("Save");
			EditSaveMResource("material_save_dlg", pMaterial);
			ShowValueEnd();
		}

		{
			MShaderMacro& shaderMacro = *pMaterial->GetShaderMacro();
			float fWidth = ImGui::GetContentRegionAvailWidth();
			if (ShowNodeBeginWithEx("Macro"))
			{
				ShowNodeExBegin("Add Macro");
				ImGui::SetNextItemWidth(fWidth * 0.7f);
				MString& addKey = GetTempValue<MString>("AddShaderMacro", "");
				EditMString(addKey);
				ImGui::SameLine();
				if (ImGui::Button("+", ImVec2(fWidth * 0.3f, 0)))
				{
					shaderMacro.AddUnionMacro(addKey);
					addKey = "";
				}

				ShowNodeExEnd();

				for (auto iter = shaderMacro.m_vMacroParams.begin(); iter != shaderMacro.m_vMacroParams.end(); ++iter)
				{
					auto& pair = *iter;

					ShowValueBegin(pair.first);
					ImGui::SetNextItemWidth(fWidth * 0.7f);
					EditMString(pair.second);
					ImGui::SameLine();
					if (ImGui::Button("Delete", ImVec2(fWidth * 0.3f, 0)))
					{
						iter = shaderMacro.m_vMacroParams.erase(iter);
					}
					ShowValueEnd();
				}

				ShowNodeEnd();
			}
		}

		{
			ShowValueBegin("Shader");
			if (ImGui::Button("Reload Shader", ImVec2(ImGui::GetContentRegionAvailWidth(), 0)))
			{
				MString strResPathVS = pResource->GetVertexShaderResource()->GetResourcePath();
				pResource->GetResourceManager()->Reload(strResPathVS);

				MString strResPathPS = pResource->GetPixelShaderResource()->GetResourcePath();
				pResource->GetResourceManager()->Reload(strResPathPS);
			}
			ShowValueEnd();
		}

		{
			ShowValueBegin("Cull");
			int nCullType = (int)pMaterial->GetRasterizerType();
			if (EditEnum({ "Wireframe", "CullNone", "CullBack", "ECullFront" }, nCullType))
			{
				pMaterial->SetRasterizerType(MERasterizerType(nCullType));
			}
			ShowValueEnd();
		}

		{
			ShowValueBegin("Type");
			int nMaterialType = (int)pMaterial->GetMaterialType();
			if (EditEnum({ "Default", "Transparent"}, nMaterialType))
			{
				pMaterial->SetMaterialType((MEMaterialType)nMaterialType);
			}
			ShowValueEnd();
		}

		{
			std::vector<MShaderParam>& vParams = *pMaterial->GetShaderParams();
			for (MShaderParam& param : vParams)
			{
				if (EditMVariant(param.strName, param.var))
				{
					param.SetDirty();
					bModified = true;
				}
			}
		}

		{
			std::vector<MShaderTextureParam>& vParams = *pMaterial->GetTextureParams();
			std::vector<MResourceKeeper>& vResources = *pMaterial->GetTextures();
			for (unsigned int i = 0; i < vParams.size(); ++i)
			{
				MShaderTextureParam& param = vParams[i];
				if (param.unCode <= SHADER_PARAM_CODE_AUTO_UPDATE)
					continue;

				MString strDlgName = "file_dlg_tex_" + MStringHelper::ToString(i);

				ShowValueBegin(param.strName);
				MResource* pResource = vResources[i].GetResource();

				EditMResource(strDlgName, pResource, MResourceManager::MEResourceType::Texture, [&param, &pMaterial](const MString& strNewFilePath) {

					MResource* pNewResource = pMaterial->GetResourceManager()->LoadResource(strNewFilePath);

					pMaterial->SetTexutreParam(param.strName, pNewResource);
					});

				if (param.pTexture)
				{
					ShowTexture(param.pTexture->GetBuffer());
				}

				ShowValueEnd();

			}
		}
	}
	

	

	return bModified;
}

void PropertyBase::EditMResource(const MString& strDlgID, MResource* pResource, const MResourceManager::MEResourceType& eResourceType, const std::function<void(const MString & strNewFilePath)>& funcLoadResource)
{

	MString strButtonLabel;
	MString strResourcePathName;
	if (pResource)
	{
		strResourcePathName = pResource->GetResourcePath();
		if (strResourcePathName.empty())
			strButtonLabel = "raw";
		else
			strButtonLabel = pResource->GetFileName(strResourcePathName);
	}
	else
	{
		strButtonLabel = strResourcePathName = "null";
	}

	bool bButtonDown = ImGui::Button(strButtonLabel.c_str(), ImVec2(ImGui::GetContentRegionAvailWidth(), 0));
	if (ImGui::IsItemHovered() && !strResourcePathName.empty())
		ImGui::SetTooltip(strResourcePathName.c_str());

	if (bButtonDown)
	{
		if (pResource)
			ImGuiFileDialog::Instance()->OpenDialog(strDlgID, vTitleList[eResourceType], vFilterList[eResourceType], pResource->GetFolder(strResourcePathName), strButtonLabel, MString(""));
		else
			ImGuiFileDialog::Instance()->OpenDialog(strDlgID, vTitleList[eResourceType], vFilterList[eResourceType], ".");
	}

	if (ImGuiFileDialog::Instance()->FileDialog(strDlgID))
	{
		if (ImGuiFileDialog::Instance()->IsOk == true)
		{
			if (funcLoadResource)
			{
				std::string filePathName = ImGuiFileDialog::Instance()->GetFilepathName();
				funcLoadResource(filePathName);
			}
		}
		ImGuiFileDialog::Instance()->CloseDialog(strDlgID);
	}
}

void PropertyBase::EditSaveMResource(const MString& stringID, MResource* pResource)
{
	if (pResource)
	{
		MString strResourcePathName = pResource->GetResourcePath();
		MString strButtonLabel = pResource->GetFileName(strResourcePathName);

		float fWidth = ImGui::GetContentRegionAvailWidth();

		if (ImGui::Button("Save", ImVec2(fWidth * 0.5f, 0)))
			pResource->Save();

		ImGui::SameLine();

		bool bButtonDown = ImGui::Button("Save To", ImVec2(fWidth * 0.5f, 0));
		if (ImGui::IsItemHovered() && !strResourcePathName.empty())
			ImGui::SetTooltip(strResourcePathName.c_str());

		if (bButtonDown)
		{
			ImGuiFileDialog::Instance()->OpenDialog(stringID, vTitleList[pResource->GetType()], vFilterList[pResource->GetType()], pResource->GetFolder(strResourcePathName), strButtonLabel, MString(""));
		}

		if (ImGuiFileDialog::Instance()->FileDialog(stringID))
		{
			if (ImGuiFileDialog::Instance()->IsOk == true)
			{
				std::string filePathName = ImGuiFileDialog::Instance()->GetFilepathName();
				pResource->SaveTo(filePathName);
			}
			ImGuiFileDialog::Instance()->CloseDialog(stringID);
		}
	}
	else
	{
		ImGui::Text("null");
	}
}

void PropertyBase::ShowTexture(MTextureBuffer* pTextureBuffer)
{
	const float fMaxImageSize = 200;
	if (pTextureBuffer)
	{
		if (pTextureBuffer->m_pShaderResourceView)
		{
			float fImageWidth = ImGui::GetContentRegionAvailWidth();
			if (fImageWidth > fMaxImageSize)
			{
				ImGui::Spacing();
				ImGui::SameLine((fImageWidth - fMaxImageSize) * 0.5f);
				ImGui::Image(ImTextureID(pTextureBuffer->m_pShaderResourceView, 0), ImVec2(fMaxImageSize, fMaxImageSize));
			}
			else
			{
				ImGui::Image(ImTextureID(pTextureBuffer->m_pShaderResourceView, 0), ImVec2(fImageWidth, fImageWidth));
			}
		}
	}
}

bool PropertyBase::EditMColor(MColor& value)
{
	return ImGui::ColorEdit4("", value.m);
}

bool PropertyBase::EditMString(MString& value)
{
	return ImGui::InputText("", &value);
}

unsigned int PropertyBase::GetID(const MString& strItemName)
{
	unsigned int unID = m_tItemID[strItemName];
	if (unID != 0)
		return unID;

	m_tItemID[strItemName] = ++m_unItemIDPool;
	return m_unItemIDPool;
}

