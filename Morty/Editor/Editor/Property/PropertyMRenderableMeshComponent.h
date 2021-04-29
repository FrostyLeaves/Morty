#ifndef _PROPERTY_RENDERABLE_MESH_COMPONENT_H_
#define _PROPERTY_RENDERABLE_MESH_COMPONENT_H_

#include "PropertyBase.h"
#include "MRenderableMeshComponent.h"

#include "imgui.h"
#include "MNode.h"
#include "MEngine.h"
#include "MMaterial.h"
#include "NotifyManager.h"

class PropertyMRenderableMeshComponent : public PropertyBase
{
public:
	virtual void EditNode(MNode* pNode) override
	{
		if (MRenderableMeshComponent* pMeshComponent = pNode->GetComponent<MRenderableMeshComponent>())
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
				EditMResource("material_file_dlg", pMaterial, MEResourceType::Material, [pNode, pMeshComponent](const MString& strNewFilePath) {
					if (MMaterial* pMaterial = dynamic_cast<MMaterial*>(pNode->GetEngine()->GetResourceManager()->LoadResource(strNewFilePath)))
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

				PROPERTY_VALUE_EDIT_SPEED_MIN_MAX(pMeshComponent, "LOD", float, GetDetailLevel, SetDetailLevel, 1, 1, MGlobal::MMESH_LOD_LEVEL_RANGE);


				ShowNodeEnd();
			}


		}
	}
};


#endif