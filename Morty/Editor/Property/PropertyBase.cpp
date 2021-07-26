#include "PropertyBase.h"

#include "MTimer.h"
#include "MMaterial.h"
#include "MTextureResource.h"
#include "MMaterialResource.h"
#include "MResourceSystem.h"

#include "imgui.h"
#include "imgui_stdlib.h"
#include "ImGuiFileDialog.h"

unsigned int PropertyBase::m_unItemIDPool = 0;
std::map<MString, unsigned int> PropertyBase::m_tItemID = std::map<MString, unsigned int>();

static const char* vTitleList[] = {
		"Default",
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
	".mvs\0.mps\0\0",
	".matl\0\0",
	".png,.jpg,.tga\0\0",
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
	case MVariant::MEVariantType::EBool:
		ShowValueBegin(strVariantName);
		bModified |= Editbool(*value.GetBool());
		ShowValueEnd();
		break;

	case MVariant::MEVariantType::EFloat:
	case MVariant::MEVariantType::EInt:
		ShowValueBegin(strVariantName);
		bModified |= Editfloat(*value.CastFloatUnsafe());
		ShowValueEnd();
		break;

	case MVariant::MEVariantType::EVector3:
		ShowValueBegin(strVariantName);
		bModified |= EditVector3(value.CastFloatUnsafe());
		ShowValueEnd();
		break;

	case MVariant::MEVariantType::EArray:
	case MVariant::MEVariantType::EStruct:
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

	case MVariant::MEVariantType::ENone:
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
			EditSaveMResource("material_save_dlg", MMaterialResource::GetResourceTypeName(), MMaterialResource::GetSuffixList(), pMaterial);
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
				pResource->GetResourceSystem()->Reload(strResPathVS);

				MString strResPathPS = pResource->GetPixelShaderResource()->GetResourcePath();
				pResource->GetResourceSystem()->Reload(strResPathPS);
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
			std::vector<MShaderConstantParam* >& vParams = *pMaterial->GetShaderParams();
			for (MShaderConstantParam* param : vParams)
			{
				if (EditMVariant(param->strName, param->var))
				{
					param->SetDirty();
					bModified = true;
				}
			}
		}

		{
			std::vector<MShaderTextureParam*>& vParams = *pMaterial->GetTextureParams();
			for (unsigned int i = 0; i < vParams.size(); ++i)
			{
				if (MShaderRefTextureParam* param = dynamic_cast<MShaderRefTextureParam*>(vParams[i]))
				{

					MString strDlgName = "file_dlg_tex_" + MStringHelper::ToString(i);

					ShowValueBegin(param->strName);
					MResource* pResource = param->m_TextureRef.GetResource();

					EditMResource(strDlgName, MTextureResource::GetResourceTypeName(), MTextureResource::GetSuffixList(), pResource, [&param, &pMaterial](const MString& strNewFilePath) {

						MResource* pNewResource = pMaterial->GetResourceSystem()->LoadResource(strNewFilePath);

						pMaterial->SetTexutreParam(param->strName, pNewResource);
						});

					if (param->pTexture)
					{
						ShowTexture(param->pTexture);
					}

					ShowValueEnd();
				}
			}
		}
	}
	

	

	return bModified;
}

void PropertyBase::EditMResource(const MString& strDlgID, const MString& strResourceType, const std::vector<MString>& vSuffixList, MResource* pResource, const std::function<void(const MString & strNewFilePath)>& funcLoadResource)
{
	//".mvs\0.mps\0\0",
	MString strSuffix = "";
	for (const MString& suffix : vSuffixList)
	{
		strSuffix += "." + suffix + "\0";
	}
	strSuffix += "\0";

	MString strButtonLabel;
	MString strResourcePathName;
	if (pResource)
	{
		strResourcePathName = pResource->GetResourcePath();
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
			ImGuiFileDialog::Instance()->OpenModal(strDlgID, strResourceType, strSuffix.c_str(), pResource->GetFolder(strResourcePathName), strButtonLabel);
		else
			ImGuiFileDialog::Instance()->OpenModal(strDlgID, strResourceType, strSuffix.c_str(), ".");
	}

	if (ImGuiFileDialog::Instance()->Display(strDlgID))
	{
		if (ImGuiFileDialog::Instance()->IsOk() == true)
		{
			if (funcLoadResource)
			{
				std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
				funcLoadResource(filePathName);
			}
		}
		ImGuiFileDialog::Instance()->Close();
	}
}

void PropertyBase::EditSaveMResource(const MString& stringID, const MString& strResourceType, const std::vector<MString>& vSuffixList, MResource* pResource)
{
	//".mvs\0.mps\0\0",
	MString strSuffix = "";
	for (const MString& suffix : vSuffixList)
	{
		strSuffix += "." + suffix + "\0";
	}
	strSuffix += "\0";

	if (pResource)
	{
		MString strResourcePathName = pResource->GetResourcePath();
		MString strButtonLabel = pResource->GetFileName(strResourcePathName);

		float fWidth = ImGui::GetContentRegionAvailWidth();

		bool bButtonDown = ImGui::Button("Save To", ImVec2(fWidth * 0.5f, 0));
		if (ImGui::IsItemHovered() && !strResourcePathName.empty())
			ImGui::SetTooltip(strResourcePathName.c_str());

		ImGui::SameLine();

		char btn_name[64];
		sprintf(btn_name, "Save##_%d", ImGui::GetID(pResource));

		if (ImGui::Button(btn_name, ImVec2(fWidth * 0.5f, 0)))
			pResource->Save();

		if (bButtonDown)
		{

			ImGuiFileDialog::Instance()->OpenModal(stringID, strResourceType, strSuffix.c_str(), pResource->GetFolder(strResourcePathName), strButtonLabel);
		}

		if (ImGuiFileDialog::Instance()->Display(stringID))
		{
			if (ImGuiFileDialog::Instance()->IsOk() == true)
			{
				std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
				pResource->SaveTo(filePathName);
			}
			ImGuiFileDialog::Instance()->Close();
		}
	}
	else
	{
		ImGui::Text("null");
	}
}

void PropertyBase::ShowTexture(MTexture* pTexture)
{
	const float fMaxImageSize = 200;
	if (pTexture)
	{
		float fImageWidth = ImGui::GetContentRegionAvailWidth();
		if (fImageWidth > fMaxImageSize)
		{
			ImGui::Spacing();
			ImGui::SameLine((fImageWidth - fMaxImageSize) * 0.5f);
			ImGui::Image(ImTextureID(pTexture), ImVec2(fMaxImageSize, fMaxImageSize));
		}
		else
		{
			ImGui::Image(ImTextureID(pTexture), ImVec2(fImageWidth, fImageWidth));
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

