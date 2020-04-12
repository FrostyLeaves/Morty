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

				ShowNodeEnd();
			}

			if (ShowNodeBegin("Material"))
			{
				MMaterial* pMaterial = pNode->GetMaterial();

				EditMMaterial(pNode->GetMaterial());

				ShowValueBegin("ShadowType");
				MIModelMeshInstance::MEShadowType eType = pNode->GetShadowType();
				unsigned int unSelected = (unsigned int)eType;
				if (EditEnum({ "None", "OnlyDirection", "AllLights" }, unSelected))
				{
					pNode->SetShadowType((MIModelMeshInstance::MEShadowType)unSelected);
				}
				ShowValueEnd();

				
				PROPERTY_VALUE_EDIT_SPEED_MIN_MAX(pNode, "LOD", float, GetDetailLevel, SetDetailLevel, 1, 1, MMESH_LOD_LEVEL_RANGE);
				


				ShowNodeEnd();
			}
		}
	}
};


#endif