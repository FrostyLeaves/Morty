#ifndef _PROPERTY_MMODELINSTANCE_H_
#define _PROPERTY_MMODELINSTANCE_H_

#include "PropertyM3DNode.h"
#include "Model/MModelInstance.h"
#include "Model/MModelResource.h"

#include "MEngine.h"
#include "MResourceManager.h"

#include "NotifyManager.h"

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
				PROPERTY_VALUE_EDIT(pNode, "DirShadow", bool, GetGenerateDirLightShadow, SetGenerateDirLightShadow);


				static auto ModelLoadFunc = [&pNode](const MString& strNewFilePath) {

					if (MResource* pNewResource = pNode->GetEngine()->GetResourceManager()->LoadResource(strNewFilePath))
					{
						pNode->Load(pNewResource);
					}

				};

				EditAnimation(pNode);

				ShowNodeEnd();
			}
		}
	}


	void EditAnimation(MModelInstance* pNode);
};


#endif