#ifndef _PROPERTY_SPOT_LIGHT_COMPONENT_H_
#define _PROPERTY_SPOT_LIGHT_COMPONENT_H_

#include "Property/PropertyBase.h"
#include "Component/MSpotLightComponent.h"

class PropertyMSpotLightComponent : public PropertyBase
{
public:
	virtual void EditEntity(MEntity* pEntity) override
	{
		if (MSpotLightComponent* pComponent = pEntity->GetComponent<MSpotLightComponent>())
		{
			if (ShowNodeBegin("Light"))
			{
				PROPERTY_VALUE_EDIT(pComponent, "Color", MColor, GetColor, SetColor);
				PROPERTY_VALUE_EDIT(pComponent, "Intensity", float, GetLightIntensity, SetLightIntensity);
				PROPERTY_VALUE_EDIT_SPEED_MIN_MAX(pComponent, "Inner CutOff", float, GetInnerCutOff, SetInnerCutOff, 1.0f, 0.0f, 180.0f);
				PROPERTY_VALUE_EDIT_SPEED_MIN_MAX(pComponent, "Outer CutOff", float, GetOuterCutOff, SetOuterCutOff, 1.0f, 0.0f, 180.0f);
	
				ShowNodeEnd();
			}
		}
	}
};

#endif