#pragma once

#include "Component/MRenderMeshComponent.h"
#include "Engine/MEngine.h"
#include "MComponentProperty.h"
#include "Main/MainEditor.h"
#include "Material/MMaterial.h"
#include "Resource/MMaterialResource.h"
#include "Resource/MMaterialResourceData.h"
#include "Scene/MEntity.h"
#include "System/MResourceSystem.h"
#include "Utility/NotifyManager.h"
#include "Widget/MaterialView.h"

namespace morty
{

class PropertyMRenderMeshComponent : public MComponentProperty
{
public:
    void EditEntity(MainEditor* editor, MEntity* pEntity) override
    {
        m_editProperty.BindEngine(pEntity->GetEngine());

        if (auto* pMeshComponent = pEntity->GetComponent<MRenderMeshComponent>())
        {
            if (m_editProperty.ShowNodeBegin("MeshComponent"))
            {
                if (m_editProperty.ShowNodeBegin("Model Mesh"))
                {
                    PROPERTY_VALUE_GET_SET_EDIT(
                            pMeshComponent,
                            "DirShadow",
                            bool,
                            GetGenerateDirLightShadow,
                            SetGenerateDirLightShadow
                    );

                    m_editProperty.ShowNodeEnd();
                }

                if (m_editProperty.ShowNodeBegin("Material"))
                {
                    m_editProperty.ShowValueBegin("Load");

                    auto pMaterialResource = pMeshComponent->GetMaterialResource();
                    if (m_editProperty.EditMResource(
                                "material_file_dlg",
                                MMaterialResourceLoader::GetResourceTypeName(),
                                MMaterialResourceLoader::GetSuffixList(),
                                pMaterialResource
                        ))
                    {
                        if (pMaterialResource) { pMeshComponent->SetMaterial(pMaterialResource); };
                    }

                    m_editProperty.ShowValueEnd();

                    m_editProperty.ShowValueBegin("Instance");
                    if (ImGui::Button("Edit Material", ImVec2(ImGui::GetContentRegionAvail().x, 0)))
                    {
                        if (pMeshComponent->GetMaterial())
                        {
                            editor->FindWidget<MaterialView>()->SetMaterial(pMeshComponent->GetMaterialResource());
                        }
                    }
                    m_editProperty.ShowValueEnd();


                    m_editProperty.ShowNodeEnd();
                }

                if (m_editProperty.ShowNodeBegin("Render"))
                {
                    m_editProperty.ShowValueBegin("ShadowType");
                    MRenderMeshComponent::MEShadowType eType     = pMeshComponent->GetShadowType();
                    auto                               nSelected = (size_t) eType;
                    if (m_editProperty.EditEnum({"None", "OnlyDirection", "AllLights"}, nSelected))
                    {
                        pMeshComponent->SetShadowType((MRenderMeshComponent::MEShadowType) nSelected);
                    }
                    m_editProperty.ShowValueEnd();

                    PROPERTY_VALUE_EDIT_SPEED_MIN_MAX(
                            pMeshComponent,
                            "LOD",
                            float,
                            GetDetailLevel,
                            SetDetailLevel,
                            1,
                            1,
                            MRenderGlobal::MESH_LOD_LEVEL_RANGE
                    );


                    m_editProperty.ShowNodeEnd();
                }

                m_editProperty.ShowNodeEnd();
            }
        }
    }
};


}// namespace morty