#pragma once

#include "Property/PropertyBase.h"
#include "Component/MPointLightComponent.h"

MORTY_SPACE_BEGIN

class PropertyMPointLightComponent : public PropertyBase
{
public:
	virtual void EditEntity(MEntity* pEntity) override
	{
		if (MPointLightComponent* pComponent = pEntity->GetComponent<MPointLightComponent>())
		{
			if (ShowNodeBegin("PointLightComponent"))
			{
				PROPERTY_VALUE_EDIT(pComponent, "Color", MColor, GetColor, SetColor);
				PROPERTY_VALUE_EDIT(pComponent, "Intensity", float, GetLightIntensity, SetLightIntensity);
				ShowNodeEnd();
			}
		}
	}
};

MORTY_SPACE_END