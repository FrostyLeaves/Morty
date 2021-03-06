#ifndef _PROPERTY_MIMODELMESHINSTANCE_H_
#define _PROPERTY_MIMODELMESHINSTANCE_H_

#include "PropertyM3DNode.h"
#include "MMaterial.h"
#include "Model/MIModelMeshInstance.h"


class PropertyMIModelMeshInstance : public PropertyM3DNode
{
public:
	virtual void EditObject(MObject* pObject) override
	{
		if (MIModelMeshInstance* pNode = pObject->DynamicCast<MIModelMeshInstance>())
		{
			if (ShowNodeBegin("Model Mesh"))
			{
				PROPERTY_VALUE_EDIT(pNode, "Draw Bounding", bool, GetDrawBoundingSphere, SetDrawBoundingSphere);
				PROPERTY_VALUE_EDIT(pNode, "DirShadow", bool, GetGenerateDirLightShadow, SetGenerateDirLightShadow);

				ShowNodeEnd();
			}

			if (ShowNodeBegin("Material"))
			{
				ShowValueBegin("Load");

				MMaterial* pMaterial = pNode->GetMaterial();
				EditMResource("material_file_dlg", pMaterial, MEResourceType::Material, [pNode](const MString& strNewFilePath) {
					if (MMaterial* pMaterial = dynamic_cast<MMaterial*>(pNode->GetEngine()->GetResourceManager()->LoadResource(strNewFilePath)))
					{
						pNode->SetMaterial(pMaterial);
					};
					});

				ShowValueEnd();

				ShowValueBegin("Instance");
				if (ImGui::Button("Edit Material", ImVec2(ImGui::GetContentRegionAvailWidth(), 0)))
				{
					int nResID = MGlobal::M_INVALID_INDEX;
					if (pNode->GetMaterial())
						nResID = pNode->GetMaterial()->GetResourceID();

					NotifyManager::GetInstance()->SendNotify("Edit Material", nResID);
				}
				ShowValueEnd();



				ShowNodeEnd();
			}

			if (ShowNodeBegin("Render"))
			{
				ShowValueBegin("ShadowType");
				MIModelMeshInstance::MEShadowType eType = pNode->GetShadowType();
				int unSelected = (int)eType;
				if (EditEnum({ "None", "OnlyDirection", "AllLights" }, unSelected))
				{
					pNode->SetShadowType((MIModelMeshInstance::MEShadowType)unSelected);
				}
				ShowValueEnd();

				PROPERTY_VALUE_EDIT_SPEED_MIN_MAX(pNode, "LOD", float, GetDetailLevel, SetDetailLevel, 1, 1, MGlobal::MMESH_LOD_LEVEL_RANGE);


				ShowNodeEnd();
			}


		}
	}
};


#endif