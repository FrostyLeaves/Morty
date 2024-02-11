#pragma once

#include "Property/PropertyBase.h"
#include "Component/MModelComponent.h"

#include "Engine/MEngine.h"

#include "Utility/NotifyManager.h"

MORTY_SPACE_BEGIN

class PropertyMModelComponent : public PropertyBase
{
public:
	virtual void EditEntity(MEntity* pEntity) override
	{
		if (MModelComponent* pModelComponent = pEntity->GetComponent<MModelComponent>())
		{
			if (ShowNodeBegin("ModelComponent"))
			{

				PROPERTY_VALUE_EDIT(pModelComponent, "Bounding", bool, GetBoundingBoxVisiable, SetBoundingBoxVisiable);

				EditAnimation(pModelComponent);

				ShowNodeEnd();
			}
		}
	}


	void EditAnimation(MModelComponent* pModelComponent);
};


MORTY_SPACE_END