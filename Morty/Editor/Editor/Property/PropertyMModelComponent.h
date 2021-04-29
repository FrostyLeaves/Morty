#ifndef _PROPERTY_MODEL_COMPONENT_H_
#define _PROPERTY_MODEL_COMPONENT_H_

#include "PropertyBase.h"
#include "MModelComponent.h"

#include "MEngine.h"
#include "MResourceManager.h"

#include "NotifyManager.h"

class PropertyMModelComponent : public PropertyBase
{
public:
	virtual void EditNode(MNode* pNode) override
	{
		if (MModelComponent* pModelComponent = pNode->GetComponent<MModelComponent>())
		{
			if (ShowNodeBegin("Model"))
			{

				PROPERTY_VALUE_EDIT(pModelComponent, "Bounding", bool, GetBoundingBoxVisiable, SetBoundingBoxVisiable);

				EditAnimation(pModelComponent);

				ShowNodeEnd();
			}
		}
	}


	void EditAnimation(MModelComponent* pModelComponent);
};


#endif