#ifndef _PROPERTY_RENDER_MESH_COMPONENT_H_
#define _PROPERTY_RENDER_MESH_COMPONENT_H_

#include "Property/PropertyBase.h"
#include "Component/MRenderMeshComponent.h"

#include "imgui.h"
#include "Scene/MEntity.h"
#include "Engine/MEngine.h"
#include "Material/MMaterial.h"
#include "Utility/NotifyManager.h"

#include "System/MResourceSystem.h"
#include "Resource/MMaterialResource.h"
#include "Resource/MMaterialResourceData.h"
#include <stdint.h>

class PropertyMRenderMeshComponent : public PropertyBase
{
public:
	virtual void EditEntity(MEntity* pEntity) override
	{

		if (MRenderMeshComponent* pMeshComponent = pEntity->GetComponent<MRenderMeshComponent>())
		{
			if (ShowNodeBegin("MeshComponent"))
			{
				if (ShowNodeBegin("Model Mesh"))
				{
					PROPERTY_VALUE_EDIT(pMeshComponent, "Draw Bounding", bool, GetDrawBoundingSphere, SetDrawBoundingSphere);
					PROPERTY_VALUE_EDIT(pMeshComponent, "DirShadow", bool, GetGenerateDirLightShadow, SetGenerateDirLightShadow);

					ShowNodeEnd();
				}

				if (ShowNodeBegin("Material"))
				{
					ShowValueBegin("Load");

					auto pMaterialResource = pMeshComponent->GetMaterialResource();
					EditMResource("material_file_dlg", MMaterialResourceLoader::GetResourceTypeName(), MMaterialResourceLoader::GetSuffixList(), pMaterialResource, [pMeshComponent](const MString& strNewFilePath) {
						MResourceSystem* pResourceSystem = pMeshComponent->GetEngine()->FindSystem<MResourceSystem>();
						if (std::shared_ptr<MMaterialResource> pMaterialResource = MTypeClass::DynamicCast<MMaterialResource>(pResourceSystem->LoadResource(strNewFilePath)))
						{
							pMeshComponent->SetMaterial(pMaterialResource);
						};
						});

					ShowValueEnd();

					ShowValueBegin("Instance");
					if (ImGui::Button("Edit Material", ImVec2(ImGui::GetContentRegionAvail().x, 0)))
					{
						int nResID = MGlobal::M_INVALID_INDEX;
						if (pMeshComponent->GetMaterial())
                        {
                            nResID = static_cast<int>(pMeshComponent->GetMaterial()->GetResourceID());
                        }
						NotifyManager::GetInstance()->SendNotify("Edit Material", MVariant(nResID));
					}
					ShowValueEnd();



					ShowNodeEnd();
				}

				if (ShowNodeBegin("Render"))
				{
					ShowValueBegin("ShadowType");
					MRenderMeshComponent::MEShadowType eType = pMeshComponent->GetShadowType();
					size_t nSelected = (size_t)eType;
					if (EditEnum({ "None", "OnlyDirection", "AllLights" }, nSelected))
					{
						pMeshComponent->SetShadowType((MRenderMeshComponent::MEShadowType)nSelected);
					}
					ShowValueEnd();

					PROPERTY_VALUE_EDIT_SPEED_MIN_MAX(pMeshComponent, "LOD", float, GetDetailLevel, SetDetailLevel, 1, 1, MRenderGlobal::MESH_LOD_LEVEL_RANGE);


					ShowNodeEnd();
				}

				ShowNodeEnd();
			}


		}
	}
};


#endif
