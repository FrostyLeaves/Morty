#ifndef _PROPERTY_MODEL_COMPONENT_H_
#define _PROPERTY_MODEL_COMPONENT_H_

#include "PropertyBase.h"
#include "MModelComponent.h"

#include "MEngine.h"

#include "NotifyManager.h"

class PropertyMModelComponent : public PropertyBase
{
public:
	virtual void EditEntity(MEntity* pEntity) override
	{
		if (MModelComponent* pModelComponent = pEntity->GetComponent<MModelComponent>())
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