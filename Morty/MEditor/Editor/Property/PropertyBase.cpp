#include "PropertyBase.h"

#include "MMaterial.h"
#include "MResource.h"
#include "MResourceManager.h"

#include "imgui.h"
#include "ImGuiFileDialog.h"

unsigned int PropertyBase::m_unItemIDPool = 0;
std::map<MString, unsigned int> PropertyBase::m_tItemID = std::map<MString, unsigned int>();

bool PropertyBase::ShowNodeBegin(const MString& strNodeName)
{
	ImGui::PushID(GetID(strNodeName));
	ImGui::AlignTextToFramePadding();
	if (bool node_open = ImGui::TreeNodeEx("Object", ImGuiTreeNodeFlags_DefaultOpen, "%s", strNodeName.c_str()))
	{
		ImGui::NextColumn();
		ImGui::AlignTextToFramePadding();
		//ImGui::Text("0.0");
		ImGui::NextColumn();

		return true;
	}

	ImGui::NextColumn();
	ImGui::NextColumn();
	ImGui::PopID();
	return false;
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

bool PropertyBase::EditEnum(const std::vector<MString>& select, unsigned int& index)
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
		bModified |= Editbool(*value.GetByType<bool>());
		ShowValueEnd();
		break;

	case MVariant::EFloat:
	case MVariant::EInt:
		ShowValueBegin(strVariantName);
		bModified |= Editfloat(*value.GetByType<float>());
		ShowValueEnd();
		break;

	case MVariant::EVector3:
		ShowValueBegin(strVariantName);
		bModified |= EditVector3(value.GetByType<float>());
		ShowValueEnd();
		break;

	case MVariant::EArray:
	case MVariant::EStruct:
	if(ShowNodeBegin(strVariantName))
	{
		MContainer* pStruct = value.GetByType<MContainer>();
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

	{
		std::vector<MShaderParam>& vParams = pMaterial->GetPixelShaderParams();
		for (MShaderParam& param : vParams)
		{
			if (param.strName.compare(0, 13, "MORTY_ENGINE_"))
			{
				if (EditMVariant(param.strName, param.var))
				{
					param.SetDirty();
					bModified = true;
				}
			}
		}
	}

	{
		std::vector<MShaderTextureParam>& vParams = pMaterial->GetPixelTextureParams();
		std::vector<MResourceHolder*>& vResources =  pMaterial->GetPixelTextures();
		for (unsigned int i = 0; i < vParams.size(); ++i)
		{
			MString strDlgName = "file_dlg_" + MStringHelper::ToString(i);

			MShaderTextureParam& param = vParams[i];

			ShowValueBegin(param.strName);
			MResource* pResource = vResources[i] ? vResources[i]->GetResource() : nullptr;

			EditMResource(strDlgName, pResource, MResourceManager::MEResourceType::Texture, [&param, &pMaterial](const MString& strNewFilePath) {

				MResource* pNewResource = pMaterial->GetResource()->GetResourceManager()->LoadResource(strNewFilePath);

				pMaterial->SetPixelTexutreParam(param.strName, pNewResource);
			});

// 			MResource* pResource = nullptr;
// 			MString strButtonLabel;
// 			MString strResourcePathName;
// 			if (vResources[i] && (pResource = vResources[i]->GetResource()))
// 			{
// 				strResourcePathName = pResource->GetResourcePath();
// 				strButtonLabel = pResource->GetFileName(strResourcePathName);
// 			}
// 			else
// 			{
// 				strButtonLabel = strResourcePathName = "null";
// 			}
// 
// 			bool bButtonDown = ImGui::Button(strButtonLabel.c_str(), ImVec2(ImGui::GetContentRegionAvailWidth(), 0));
// 			if (ImGui::IsItemHovered())
// 				ImGui::SetTooltip(strResourcePathName.c_str());
// 
// 			if (bButtonDown)
// 			{
// 				if (pResource)
// 					ImGuiFileDialog::Instance()->OpenDialog(strDlgName, "Choose Texture", ".png\0.jpg\0.tga\0\0", pResource->GetFolder(strResourcePathName), strButtonLabel, "");
// 				else
// 					ImGuiFileDialog::Instance()->OpenDialog(strDlgName, "Choose Texture", ".png\0.jpg\0.tga\0\0", ".");
// 			}
// 			if (ImGuiFileDialog::Instance()->FileDialog(strDlgName))
// 			{
// 				if (ImGuiFileDialog::Instance()->IsOk == true)
// 				{
// 					std::string filePathName = ImGuiFileDialog::Instance()->GetFilepathName();
// 					std::string filePath = ImGuiFileDialog::Instance()->GetCurrentPath();
// 
// 					MResource* pNewResource = pMaterial->GetResource()->GetResourceManager()->LoadResource(filePathName);
// 
// 					pMaterial->SetPixelTexutreParam(param.strName, pNewResource);
// 				}
// 				ImGuiFileDialog::Instance()->CloseDialog(strDlgName);
// 			}
			ShowValueEnd();

		}
	}


	return bModified;
}

void PropertyBase::EditMResource(const MString& strDlgID, MResource* pResource, const MResourceManager::MEResourceType& eResourceType, std::function<void(const MString & strNewFilePath)> funcLoadResource)
{
	static const char* vTitleList[] = {
		"Default",
		"Model",
		"Shader",
		"Material",
		"Texture",
	};

	static const char* vFilterList[] = {
		"\0\0",
		".fbx\0.obj\0.dae\0\0",
		".mvs\0.mps\0\0",
		"\0\0",
		".png\0.jpg\0.tga\0\0",
	};

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
	if (ImGui::IsItemHovered())
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

bool PropertyBase::EditMColor(MColor& value)
{
	return ImGui::ColorEdit4("", value.m);
}

unsigned int PropertyBase::GetID(const MString& strItemName)
{
	unsigned int unID = m_tItemID[strItemName];
	if (unID != 0)
		return unID;

	m_tItemID[strItemName] = ++m_unItemIDPool;
	return m_unItemIDPool;
}

