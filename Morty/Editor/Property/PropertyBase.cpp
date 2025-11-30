#include "Property/PropertyBase.h"
#include "Engine/MEngine.h"
#include "ImGuiFileDialog.h"
#include "Material/MMaterial.h"
#include "Resource/MMaterialResource.h"
#include "Resource/MMaterialResourceData.h"
#include "Resource/MMaterialTemplateResource.h"
#include "Resource/MMaterialTemplateResourceData.h"
#include "Resource/MTextureResource.h"
#include "System/MResourceSystem.h"
#include "Utility/MTimer.h"
#include "imgui.h"
#include "imgui_stdlib.h"

using namespace morty;

unsigned int                    PropertyBase::m_unItemIDPool = 0;
std::map<MString, unsigned int> PropertyBase::m_itemID       = std::map<MString, unsigned int>();

bool                            PropertyBase::ShowNodeBegin(const MString& strNodeName)
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
    if (ImGui::TreeNodeEx("Object", ImGuiTreeNodeFlags_DefaultOpen, "%s", strNodeName.c_str()))
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

void PropertyBase::ShowNodeExBegin(const MString& strExID) { ImGui::PushID(GetID(strExID)); }

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
    ImGui::TreeNodeEx(
            strValueName.c_str(),
            ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Bullet
    );
    ImGui::NextColumn();
    ImGui::SetNextItemWidth(-1);
}

void PropertyBase::ShowValueEnd()
{
    ImGui::NextColumn();
    ImGui::PopID();
}

bool PropertyBase::Editbool(bool& value) { return ImGui::Checkbox("", &value); }

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

bool PropertyBase::EditVector2(
        Vector2&     value,
        const float& fSpeed /*= 1.0f*/,
        const float& fMin /*= 0.0f*/,
        const float& fMax /*= 0.0f*/
)
{
    if (value.x == -0.0f) value.x = 0.0f;
    if (value.y == -0.0f) value.y = 0.0f;
    return ImGui::DragFloat2("", value.m, fSpeed, fMin, fMax);
}

bool PropertyBase::EditVector3(Vector3& value, const float& fSpeed, const float& fMin, const float& fMax)
{
    return EditVector3(value.m, fSpeed, fMin, fMax);
}

bool PropertyBase::EditVector3(
        float*       pValue,
        const float& fSpeed /*= 1.0f*/,
        const float& fMin /*= 0.0f*/,
        const float& fMax /*= 0.0f*/
)
{
    if (pValue[0] == -0.0f) pValue[0] = 0.0f;
    if (pValue[1] == -0.0f) pValue[1] = 0.0f;
    if (pValue[2] == -0.0f) pValue[2] = 0.0f;
    return ImGui::DragFloat3("", pValue, fSpeed, fMin, fMax);
}

bool PropertyBase::EditVector4(Vector4& value, const float& fSpeed, const float& fMin, const float& fMax)
{
    return EditVector4(value.m, fSpeed, fMin, fMax);
}

bool PropertyBase::EditVector4(float* pValue, const float& fSpeed, const float& fMin, const float& fMax)
{
    if (pValue[0] == -0.0f) pValue[0] = 0.0f;
    if (pValue[1] == -0.0f) pValue[1] = 0.0f;
    if (pValue[2] == -0.0f) pValue[2] = 0.0f;
    return ImGui::DragFloat4("", pValue, fSpeed, fMin, fMax);
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

bool PropertyBase::EditEnum(const std::vector<MString>& select, size_t& index)
{
    if (index >= select.size()) { return false; }

    size_t nNewIndex = index;
    if (ImGui::BeginCombo("", select[index].c_str()))
    {

        for (size_t i = 0; i < select.size(); ++i)
        {
            if (ImGui::Selectable(select[i].c_str(), (index == i))) { nNewIndex = i; }
        }

        ImGui::EndCombo();
    }

    if (nNewIndex != index)
    {
        index = nNewIndex;
        return true;
    }

    return false;
}

bool PropertyBase::EditEnumTable(const std::map<MString, int>& select, int& index)
{
    if (select.empty()) { return false; }

    // Find current selection name
    MString currentName = "Unknown";
    for (const auto& pair: select)
    {
        if (pair.second == index)
        {
            currentName = pair.first;
            break;
        }
    }

    int nNewIndex = index;
    if (ImGui::BeginCombo("", currentName.c_str()))
    {
        for (const auto& pair: select)
        {
            const bool isSelected = (pair.second == index);
            if (ImGui::Selectable(pair.first.c_str(), isSelected)) { nNewIndex = pair.second; }

            // Set the initial focus when opening the combo
            if (isSelected) { ImGui::SetItemDefaultFocus(); }
        }

        ImGui::EndCombo();
    }

    if (nNewIndex != index)
    {
        index = nNewIndex;
        return true;
    }

    return false;
}

bool PropertyBase::EditMVariant(MVariant& value)
{
    bool bModified = false;
    switch (value.GetType())
    {
        case MEVariantType::EUInt: {
            float val = value.GetValue<uint32_t>();
            bModified |= Editfloat(val, 1.0f, 0.0f);
            value.SetValue<uint32_t>(val);
        }
        break;

        case MEVariantType::EInt: {
            float val = static_cast<float>(value.GetValue<int>());
            bModified |= Editfloat(val, 1.0f);
            value.SetValue<int>(val);
        }
        break;

        case MEVariantType::EFloat: bModified |= Editfloat(value.GetValue<float>()); break;
        case MEVariantType::EVector2: bModified |= EditVector2(value.GetValue<Vector2>()); break;

        case MEVariantType::EVector3: bModified |= EditVector3(value.GetValue<Vector3>()); break;

        case MEVariantType::EVector4: bModified |= EditVector4(value.GetValue<Vector4>()); break;

        case MEVariantType::EArray:
        case MEVariantType::EStruct: {
            MVariantStruct& sut    = value.GetValue<MVariantStruct>();
            size_t          nCount = 0;
            for (auto& iter: sut.GetMember())
            {
                bModified |= EditMVariant(
                        iter.first.ToString().empty() ? MStringUtil::ToString(nCount) : iter.first.ToString(),
                        sut.GetVariant<MVariant>(iter.first)
                );
                nCount++;
            }

            break;
        }

        case MEVariantType::ENone:
        default: break;
    }

    return bModified;
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
        case MEVariantType::EVector2:
            ShowValueBegin(strVariantName);
            bModified |= EditVector2(value.GetValue<Vector2>());
            ShowValueEnd();
            break;

        case MEVariantType::EVector3:
            ShowValueBegin(strVariantName);
            bModified |= EditVector3(value.GetValue<Vector3>());
            ShowValueEnd();
            break;

        case MEVariantType::EVector4:
            ShowValueBegin(strVariantName);
            bModified |= EditVector4(value.GetValue<Vector4>());
            ShowValueEnd();
            break;

        case MEVariantType::EArray:
        case MEVariantType::EStruct:
            if (ShowNodeBegin(strVariantName))
            {
                MVariantStruct& sut    = value.GetValue<MVariantStruct>();
                size_t          nCount = 0;
                for (auto& iter: sut.GetMember())
                {
                    bModified |= EditMVariant(
                            iter.first.ToString().empty() ? MStringUtil::ToString(nCount) : iter.first.ToString(),
                            sut.GetVariant<MVariant>(iter.first)
                    );
                    nCount++;
                }

                ShowNodeEnd();
                break;
            }

        case MEVariantType::ENone:
        default: break;
    }

    return bModified;
}

bool PropertyBase::EditMMaterialTemplate(const std::shared_ptr<MMaterialTemplate>& material)
{
    bool bModified = false;

    {
        bool         bModify     = false;
        MShaderMacro shaderMacro = material->GetShaderMacro();
        float        fWidth      = ImGui::GetContentRegionAvail().x;
        if (ShowNodeBeginWithEx("Macro"))
        {
            ShowNodeExBegin("Add Macro");
            ImGui::SetNextItemWidth(fWidth * 0.7f);
            static MString addKey;
            EditMString(addKey);
            ImGui::SameLine();
            if (ImGui::Button("+", ImVec2(fWidth * 0.3f, 0)))
            {
                shaderMacro.AddUnionMacro(MStringId(addKey));
                addKey  = "";
                bModify = true;
            }

            ShowNodeExEnd();

            for (auto iter = shaderMacro.m_macroParams.begin(); iter != shaderMacro.m_macroParams.end(); ++iter)
            {
                auto& pair = *iter;

                ShowValueBegin(pair.first.ToString());
                ImGui::SetNextItemWidth(fWidth * 0.7f);
                EditMString(pair.second);
                ImGui::SameLine();
                if (ImGui::Button("Delete", ImVec2(fWidth * 0.3f, 0)))
                {
                    iter    = shaderMacro.m_macroParams.erase(iter);
                    bModify = true;
                }
                ShowValueEnd();
            }

            ShowNodeEnd();

            if (bModify) { material->SetShaderMacro(shaderMacro); }
        }
    }

    {
        ShowValueBegin("Shader");
        if (ImGui::Button("Reload Shader", ImVec2(ImGui::GetContentRegionAvail().x, 0)))
        {
            MString strResPath = material->GetShaderResource()->GetResourcePath();
            material->GetResourceSystem()->Reload(strResPath);
        }
        ShowValueEnd();
    }

    for (const auto& pair: material->GetPasses())
    {
        //pass name
        ImGui::TextUnformatted(pair.first.ToString().c_str());

        if (auto pass = pair.second.get())
        {
            {
                ShowValueBegin("Cull");
                auto nCullType = static_cast<size_t>(pass->GetCullMode());
                if (EditEnum({"Wireframe", "CullNone", "CullBack", "ECullFront"}, nCullType))
                {
                    pass->SetCullMode(MECullMode(nCullType));
                }
                ShowValueEnd();
            }
        }
    }

    return bModified;
}

bool PropertyBase::EditMMaterial(const std::shared_ptr<MMaterial>& material)
{
    bool bModified = false;
    if (!material) { return false; }

    {
        ShowValueBegin("Save");
        EditSaveMResource(
                "material_save_dlg",
                MMaterialResourceLoader::GetResourceTypeName(),
                MMaterialResourceLoader::GetSuffixList(),
                material
        );
        ShowValueEnd();
    }

    auto materialTemplate = MTypeClass::DynamicCast<MMaterialTemplateResource>(material->GetTemplate());
    ShowValueBegin("Material Template");
    if (EditMMaterialTemplateResource(materialTemplate))
    {
        material->ResetMaterialTemplate(materialTemplate);
        bModified = true;
    }
    ShowValueEnd();

    bModified |= EditMaterialProperty(material->GetPropertyModifier());

    {
        auto& resourceParams = material->GetPropertyModifier()->GetModifiedResources();
        for (auto& resourcepParam: resourceParams)
        {
            if (auto* param = dynamic_cast<MTextureResourceParam*>(resourcepParam.second.param))
            {
                MString strDlgName = MString("file_dlg_tex_") + resourcepParam.first.ToString();

                ShowValueBegin(param->strName.ToString());
                std::shared_ptr<MTextureResource> pTextureResource = param->GetTextureResource();

                if (auto pPreviewTexture = param->GetTexture())
                {
                    const ImGuiStyle& style = ImGui::GetStyle();
                    float             fSize = ImGui::GetFontSize() + style.FramePadding.y * 2;
                    ShowTexture(pPreviewTexture, Vector2(fSize, fSize));
                    if (ImGui::IsItemHovered())
                    {
                        ImGui::BeginTooltip();
                        ShowTexture(pPreviewTexture, Vector2(128, 128));
                        ImGui::EndTooltip();
                    }
                    ImGui::SameLine();
                }

                if (EditMResource(
                            strDlgName,
                            MTextureResourceLoader::GetResourceTypeName(),
                            MTextureResourceLoader::GetSuffixList(),
                            pTextureResource
                    ))
                {
                    material->SetTexture(param->strName, pTextureResource);
                }

                ShowValueEnd();
            }
        }
    }

    return bModified;
}

bool PropertyBase::EditMaterialProperty(MMaterialPropertyModifier* modifier)
{
    bool bModified = false;
    for (auto& param: modifier->GetModifiedParams())
    {
        if (EditMVariant(param.first.ToString(), param.second.value))
        {
            param.second.param->SetDirty();
            bModified = true;
        }
    }

    return bModified;
}

bool PropertyBase::EditMMaterialTemplateResource(std::shared_ptr<MMaterialTemplateResource>& resource)
{
    auto dlgId = std::to_string(reinterpret_cast<uint64_t>(resource.get()));

    return EditMResource(
            dlgId,
            MMaterialTemplateResourceDataLoader::GetResourceTypeName(),
            MMaterialTemplateResourceDataLoader::GetSuffixList(),
            resource
    );
}

bool PropertyBase::EditMMaterialResource(std::shared_ptr<MMaterialResource>& resource)
{
    auto dlgId = std::to_string(reinterpret_cast<uint64_t>(resource.get()));

    return EditMResource(
            dlgId,
            MMaterialResourceLoader::GetResourceTypeName(),
            MMaterialResourceLoader::GetSuffixList(),
            resource
    );
}

bool PropertyBase::EditMResource(
        const MString&              strDlgID,
        const MString&              strResourceType,
        const std::vector<MString>& vSuffixList,
        std::shared_ptr<MResource>  pResource
)
{
    if (m_engine == nullptr) { return false; }

    bool    modifyFlag     = false;
    auto    resourceSystem = m_engine->FindSystem<MResourceSystem>();

    //".mvs\0.mps\0\0",
    MString strSuffix;
    for (const MString& suffix: vSuffixList) { strSuffix += "." + suffix + ","; }
    strSuffix += '\0';

    MString strButtonLabel;
    MString strResourcePathName;
    if (pResource)
    {
        strResourcePathName = pResource->GetResourcePath();
        strButtonLabel      = MResource::GetFileName(strResourcePathName);
    }
    else { strButtonLabel = strResourcePathName = "null"; }

    bool bButtonDown = ImGui::Button(strButtonLabel.c_str(), ImVec2(ImGui::GetContentRegionAvail().x, 0));
    if (ImGui::IsItemHovered() && !strResourcePathName.empty())
    {
        ImGui::SetTooltip("%s", strResourcePathName.c_str());
    }

    if (bButtonDown)
    {
        if (pResource)
            ImGuiFileDialog::Instance()->OpenModal(
                    strDlgID,
                    strResourceType,
                    strSuffix.c_str(),
                    MResource::GetFolder(strResourcePathName),
                    strButtonLabel
            );
        else
            ImGuiFileDialog::Instance()->OpenModal(strDlgID, strResourceType, strSuffix.c_str(), ".");
    }

    if (ImGuiFileDialog::Instance()->Display(strDlgID))
    {
        if (ImGuiFileDialog::Instance()->IsOk())
        {
            std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
            if (MFileHelper::IsExist(filePathName))
            {
                pResource  = resourceSystem->LoadResource(filePathName);
                modifyFlag = true;
            }
        }
        ImGuiFileDialog::Instance()->Close();
    }

    return modifyFlag;
}

void PropertyBase::EditSaveMResource(
        const MString&              stringID,
        const MString&              strResourceType,
        const std::vector<MString>& vSuffixList,
        std::shared_ptr<MResource>  pResource
)
{
    //".mvs\0.mps\0\0",
    MString strSuffix = "";
    for (const MString& suffix: vSuffixList) { strSuffix += "." + suffix + "\0"; }
    strSuffix += "\0";

    if (pResource)
    {
        MString strResourcePathName = pResource->GetResourcePath();
        MString strButtonLabel      = pResource->GetFileName(strResourcePathName);

        float   fWidth = ImGui::GetContentRegionAvail().x;

        bool    bButtonDown = ImGui::Button("Save To", ImVec2(fWidth * 0.5f, 0));
        if (ImGui::IsItemHovered() && !strResourcePathName.empty())
        {
            ImGui::SetTooltip("%s", strResourcePathName.c_str());
        }

        ImGui::SameLine();

        MString btn_name = fmt::format("Save##_{}", ImGui::GetID(pResource.get()));

        if (ImGui::Button(btn_name.c_str(), ImVec2(fWidth * 0.5f, 0)))
        {
            auto resourceSystem = pResource->GetEngine()->FindSystem<MResourceSystem>();
            resourceSystem->SaveResource(pResource);
        }

        if (bButtonDown)
        {

            ImGuiFileDialog::Instance()->OpenModal(
                    stringID,
                    strResourceType,
                    strSuffix.c_str(),
                    pResource->GetFolder(strResourcePathName),
                    strButtonLabel
            );
        }

        if (ImGuiFileDialog::Instance()->Display(stringID))
        {
            if (ImGuiFileDialog::Instance()->IsOk() == true)
            {
                std::string filePathName   = ImGuiFileDialog::Instance()->GetFilePathName();
                auto        resourceSystem = pResource->GetEngine()->FindSystem<MResourceSystem>();
                resourceSystem->MoveTo(pResource, filePathName);
                resourceSystem->SaveResource(pResource);
            }
            ImGuiFileDialog::Instance()->Close();
        }
    }
    else { ImGui::Text("null"); }
}

void PropertyBase::ShowTexture(MTexturePtr texture, const Vector2& v2Size)
{
    if (texture) { ImGui::Image({texture, intptr_t(texture.get()), 0}, ImVec2(v2Size.x, v2Size.y)); }
}

bool         PropertyBase::EditMColor(MColor& value) { return ImGui::ColorEdit4("", value.m); }

bool         PropertyBase::EditMString(MString& value) { return ImGui::InputText("", &value); }

unsigned int PropertyBase::GetID(const MString& strItemName)
{
    unsigned int unID = m_itemID[strItemName];
    if (unID != 0) return unID;

    m_itemID[strItemName] = ++m_unItemIDPool;
    return m_unItemIDPool;
}
