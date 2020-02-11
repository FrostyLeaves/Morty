#ifndef _PROPERTY_MMODELINSTANCE_H_
#define _PROPERTY_MMODELINSTANCE_H_

#include "PropertyM3DNode.h"
#include "MModelInstance.h"
#include "MModelResource.h"

#include "MEngine.h"
#include "MResourceManager.h"

class PropertyMModelInstance : public PropertyM3DNode
{
public:
	virtual void EditObject(MObject* pObject) override
	{
		if (MModelInstance* pNode = pObject->DynamicCast<MModelInstance>())
		{
			if (ShowNodeBegin("Model"))
			{

				PROPERTY_VALUE_EDIT(pNode, "Bounding", bool, GetDrawBoundingBox, SetDrawBoundingBox);


				MModelResource* pResource = pNode->GetResource();
				ShowValueBegin("Resource");
				EditMResource("model_file_dlg", pResource, MResourceManager::MEResourceType::Model, [&pNode](const MString& strNewFilePath) {

					if (MResource* pNewResource = pNode->GetEngine()->GetResourceManager()->LoadResource(strNewFilePath))
					{
						pNode->Load(pNewResource);
					}

				});

				ShowValueEnd();

				EditAnimation(pNode);

				ShowNodeEnd();
			}
		}
	}


	void EditAnimation(MModelInstance* pNode);
};


#endif