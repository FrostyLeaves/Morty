#ifndef _PROPERTY_MPOINT_LIGHT_COMPONENT_H_
#define _PROPERTY_MPOINTLIGHT_H_

#include "Property/PropertyBase.h"
#include "Component/MPointLightComponent.h"

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


#endif