#include "Property/PropertyBase.h"

#include "Utility/MTimer.h"
#include "Material/MMaterial.h"
#include "Resource/MTextureResource.h"
#include "Resource/MMaterialResource.h"
#include "System/MResourceSystem.h"

#include "imgui.h"
#include "imgui_stdlib.h"
#include "ImGuiFileDialog.h"
#include "Engine/MEngine.h"
#include "Resource/MMaterialResourceData.h"


unsigned int PropertyBase::m_unItemIDPool = 0;
std::map<MString, unsigned int> PropertyBase::m_tItemID = std::map<MString, unsigned int>();

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
	Vector3 rotate = GetTemporaryValue<Vector3>("Transform_Rotate", trans.GetRotation().GetEulerAngle());

	if (EditVector3(rotate, 1.0f, -360.0f, 360.0f))
	{
		SetTemporaryValue("Transform_Rotate", rotate);

		Quaternion quat;
		quat.SetEulerAngle(rotate);
		quat.Normalize();
		trans.SetRotation(quat);

		bModify = true;
	}
	ShowValueEnd();

	return bModify;
}

bool PropertyBase::EditEnum(const std::vector<MString>& select, uint32_t& index)
{
	if (index >= select.size())
	{
		return false;
	}

	uint32_t unNewIndex = index;
	if (ImGui::BeginCombo("", select[index].c_str())) {

		for (size_t i = 0; i < select.size(); ++i)
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
	case MEVariantType::EUInt:
		ShowValueBegin(strVariantName);
		{
			float val = value.GetValue<uint32_t>();
			bModified |= Editfloat(val, 1.0f, 0.0f);
			value.SetValue<uint32_t>(val);
		}
		ShowValueEnd();
		break;

	case MEVariantType::EInt:
		ShowValueBegin(strVariantName);
		{
			float val = static_cast<float>(value.GetValue<int>());
			bModified |= Editfloat(val, 1.0f);
			value.SetValue<int>(val);
		}
		ShowValueEnd();
		break;

	case MEVariantType::EFloat:
		ShowValueBegin(strVariantName);
		bModified |= Editfloat(value.GetValue<float>());
		ShowValueEnd();
		break;

	case MEVariantType::EVector3:
		ShowValueBegin(strVariantName);
		bModified |= EditVector3(value.GetValue<Vector3>());
		ShowValueEnd();
		break;

	case MEVariantType::EArray:
	case MEVariantType::EStruct:
	if(ShowNodeBegin(strVariantName))
	{
		MVariantStruct& sut = value.GetValue<MVariantStruct>();
		size_t nCount = 0;
		for (auto& iter : sut.GetMember())
		{
			bModified |= EditMVariant(iter.first.ToString().empty() ? MStringUtil::ToString(nCount) : iter.first.ToString(), sut.GetVariant<MVariant>(iter.first));
			nCount++;
		}

		ShowNodeEnd();
		break;
	}

	case MEVariantType::ENone:
	default:
		break;
	}

	return bModified;
}

bool PropertyBase::EditMMaterial(std::shared_ptr<MMaterial> pMaterial)
{
	bool bModified = false;

	
	if (std::shared_ptr<MMaterial> pResource = pMaterial)
	{
		{
			ShowValueBegin("Save");
			EditSaveMResource("material_save_dlg", MMaterialResourceLoader::GetResourceTypeName(), MMaterialResourceLoader::GetSuffixList(), pMaterial);
			ShowValueEnd();
		}

		{
			MShaderMacro& shaderMacro = pMaterial->GetShaderMacro();
			float fWidth = ImGui::GetContentRegionAvail().x;
			if (ShowNodeBeginWithEx("Macro"))
			{
				ShowNodeExBegin("Add Macro");
				ImGui::SetNextItemWidth(fWidth * 0.7f);
				static MString addKey = "";
				EditMString(addKey);
				ImGui::SameLine();
				if (ImGui::Button("+", ImVec2(fWidth * 0.3f, 0)))
				{
					shaderMacro.AddUnionMacro(MStringId(addKey.c_str()));
					addKey = "";
				}

				ShowNodeExEnd();

				for (auto iter = shaderMacro.m_vMacroParams.begin(); iter != shaderMacro.m_vMacroParams.end(); ++iter)
				{
					auto& pair = *iter;

					ShowValueBegin(pair.first.ToString());
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
			if (ImGui::Button("Reload Shader", ImVec2(ImGui::GetContentRegionAvail().x, 0)))
			{
				MString strResPathVS = pResource->GetShader(MEShaderType::EVertex)->GetResourcePath();
				pResource->GetResourceSystem()->Reload(strResPathVS);

				MString strResPathPS = pResource->GetShader(MEShaderType::EPixel)->GetResourcePath();
				pResource->GetResourceSystem()->Reload(strResPathPS);
			}
			ShowValueEnd();
		}

		{
			ShowValueBegin("Cull");
			uint32_t nCullType = (uint32_t)pMaterial->GetCullMode();
			if (EditEnum({ "Wireframe", "CullNone", "CullBack", "ECullFront" }, nCullType))
			{
				pMaterial->SetCullMode(MECullMode(nCullType));
			}
			ShowValueEnd();
		}

		{
			ShowValueBegin("Type");
			uint32_t nMaterialType = (uint32_t)pMaterial->GetMaterialType();
			if (EditEnum({ "Default", "Transparent"}, nMaterialType))
			{
				pMaterial->SetMaterialType((MEMaterialType)nMaterialType);
			}
			ShowValueEnd();
		}

		{
			const std::vector<std::shared_ptr<MShaderConstantParam>>& vParams = pMaterial->GetShaderParams();
			for (const std::shared_ptr<MShaderConstantParam>& param : vParams)
			{
				if (EditMVariant(param->strName.ToString(), param->var))
				{
					param->SetDirty();
					bModified = true;
				}
			}
		}

		{
			std::vector< std::shared_ptr<MShaderTextureParam>>& vParams = pMaterial->GetTextureParams();
			for (unsigned int i = 0; i < vParams.size(); ++i)
			{
				if (const std::shared_ptr<MTextureResourceParam>& param = std::dynamic_pointer_cast<MTextureResourceParam>(vParams[i]))
				{

					MString strDlgName = "file_dlg_tex_" + MStringUtil::ToString(i);

					ShowValueBegin(param->strName.ToString());
					std::shared_ptr<MTextureResource> pResource = param->GetTextureResource();

					if (auto pPreviewTexture = param->GetTexture())
					{
						const ImGuiStyle& style = ImGui::GetStyle();
						float fSize = ImGui::GetFontSize() + style.FramePadding.y * 2;
						ShowTexture(pPreviewTexture, Vector2(fSize, fSize));
						if (ImGui::IsItemHovered())
						{
							ImGui::BeginTooltip();
							ShowTexture(pPreviewTexture, Vector2(128, 128));
							ImGui::EndTooltip();
						}
						ImGui::SameLine();
					}

					EditMResource(strDlgName, MTextureResourceLoader::GetResourceTypeName(), MTextureResourceLoader::GetSuffixList(), pResource, [&param, &pMaterial](const MString& strNewFilePath) {

						std::shared_ptr<MResource> pNewResource = pMaterial->GetResourceSystem()->LoadResource(strNewFilePath);

						pMaterial->SetTexture(param->strName, pNewResource);
						});

					ShowValueEnd();
				}
			}
		}
	}
	

	

	return bModified;
}

void PropertyBase::EditMResource(const MString& strDlgID, const MString& strResourceType, const std::vector<MString>& vSuffixList, std::shared_ptr<MResource> pResource, const std::function<void(const MString & strNewFilePath)>& funcLoadResource)
{
	//".mvs\0.mps\0\0",
	MString strSuffix = "";
	for (const MString& suffix : vSuffixList)
	{
		strSuffix += "." + suffix + ",";
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

	bool bButtonDown = ImGui::Button(strButtonLabel.c_str(), ImVec2(ImGui::GetContentRegionAvail().x, 0));
	if (ImGui::IsItemHovered() && !strResourcePathName.empty()) {
		ImGui::SetTooltip("%s", strResourcePathName.c_str());
	}

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

void PropertyBase::EditSaveMResource(const MString& stringID, const MString& strResourceType, const std::vector<MString>& vSuffixList, std::shared_ptr<MResource> pResource)
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

		float fWidth = ImGui::GetContentRegionAvail().x;

		bool bButtonDown = ImGui::Button("Save To", ImVec2(fWidth * 0.5f, 0));
		if (ImGui::IsItemHovered() && !strResourcePathName.empty()) {
			ImGui::SetTooltip("%s", strResourcePathName.c_str());
		}

		ImGui::SameLine();

		char btn_name[64];
		sprintf(btn_name, "Save##_%u", ImGui::GetID(pResource.get()));

		if (ImGui::Button(btn_name, ImVec2(fWidth * 0.5f, 0)))
		{
			auto pResourceSystem = pResource->GetEngine()->FindSystem<MResourceSystem>();
			pResourceSystem->SaveResource(pResource);
		}

		if (bButtonDown)
		{

			ImGuiFileDialog::Instance()->OpenModal(stringID, strResourceType, strSuffix.c_str(), pResource->GetFolder(strResourcePathName), strButtonLabel);
		}

		if (ImGuiFileDialog::Instance()->Display(stringID))
		{
			if (ImGuiFileDialog::Instance()->IsOk() == true)
			{
				std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
				auto pResourceSystem = pResource->GetEngine()->FindSystem<MResourceSystem>();
				pResourceSystem->MoveTo(pResource, filePathName);
				pResourceSystem->SaveResource(pResource);
			}
			ImGuiFileDialog::Instance()->Close();
		}
	}
	else
	{
		ImGui::Text("null");
	}
}

void PropertyBase::ShowTexture(std::shared_ptr<MTexture> pTexture, const Vector2& v2Size)
{
	if (pTexture)
	{
		ImGui::Image({ pTexture, intptr_t(pTexture.get()), 0 }, ImVec2(v2Size.x, v2Size.y));
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

