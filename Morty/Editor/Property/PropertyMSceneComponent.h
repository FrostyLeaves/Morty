#pragma once

#include "Property/PropertyBase.h"
#include "Component/MSceneComponent.h"

MORTY_SPACE_BEGIN

class PropertyMSceneComponent : public PropertyBase
{
public:
	virtual void EditEntity(MEntity* pEntity) override
	{
		if (MSceneComponent* pSceneComponent = pEntity->GetComponent<MSceneComponent>())
		{
			if (ShowNodeBegin("SceneComponent"))
			{
				PROPERTY_NODE_EDIT(pSceneComponent, "Transform", MTransform, GetTransform, SetTransform);
				PROPERTY_VALUE_EDIT(pSceneComponent, "Visible", bool, GetVisible, SetVisible);
				ShowNodeEnd();
			}
		}
	}
};

MORTY_SPACE_END