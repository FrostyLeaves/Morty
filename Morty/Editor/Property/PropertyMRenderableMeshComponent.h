#ifndef _PROPERTY_RENDERABLE_MESH_COMPONENT_H_
#define _PROPERTY_RENDERABLE_MESH_COMPONENT_H_

#include "PropertyBase.h"
#include "MRenderableMeshComponent.h"

#include "imgui.h"
#include "MEntity.h"
#include "MEngine.h"
#include "MMaterial.h"
#include "NotifyManager.h"

#include "MResourceSystem.h"
#include "MMaterialResource.h"

class PropertyMRenderableMeshComponent : public PropertyBase
{
public:
	virtual void EditEntity(MEntity* pEntity) override
	{

		if (MRenderableMeshComponent* pMeshComponent = pEntity->GetComponent<MRenderableMeshComponent>())
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

					MMaterial* pMaterial = pMeshComponent->GetMaterial();
					EditMResource("material_file_dlg", MMaterialResource::GetResourceTypeName(), MMaterialResource::GetSuffixList(), pMaterial, [pMeshComponent](const MString& strNewFilePath) {
						MResourceSystem* pResourceSystem = pMeshComponent->GetEngine()->FindSystem<MResourceSystem>();
						if (MMaterial* pMaterial = dynamic_cast<MMaterial*>(pResourceSystem->LoadResource(strNewFilePath)))
						{
							pMeshComponent->SetMaterial(pMaterial);
						};
						});

					ShowValueEnd();

					ShowValueBegin("Instance");
					if (ImGui::Button("Edit Material", ImVec2(ImGui::GetContentRegionAvailWidth(), 0)))
					{
						int nResID = MGlobal::M_INVALID_INDEX;
						if (pMeshComponent->GetMaterial())
							nResID = pMeshComponent->GetMaterial()->GetResourceID();

						NotifyManager::GetInstance()->SendNotify("Edit Material", nResID);
					}
					ShowValueEnd();



					ShowNodeEnd();
				}

				if (ShowNodeBegin("Render"))
				{
					ShowValueBegin("ShadowType");
					MRenderableMeshComponent::MEShadowType eType = pMeshComponent->GetShadowType();
					int unSelected = (int)eType;
					if (EditEnum({ "None", "OnlyDirection", "AllLights" }, unSelected))
					{
						pMeshComponent->SetShadowType((MRenderableMeshComponent::MEShadowType)unSelected);
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