#ifndef _PROPERTY_MDIRECTIONALLIGHT_H_
#define _PROPERTY_MDIRECTIONALLIGHT_H_

#include "Property/PropertyBase.h"
#include "Component/MDirectionalLightComponent.h"



class PropertyMDirectionalLightComponent : public PropertyBase
{
public:
	virtual void EditEntity(MEntity* pEntity) override
	{
		if (MDirectionalLightComponent* pDirectonalLightComponent = pEntity->GetComponent<MDirectionalLightComponent>())
		{
			if (ShowNodeBegin("DirLightComponent"))
			{
				PROPERTY_VALUE_EDIT(pDirectonalLightComponent, "Color", MColor, GetColor, SetColor);
				PROPERTY_VALUE_EDIT(pDirectonalLightComponent, "Intensity", float, GetLightIntensity, SetLightIntensity);
				PROPERTY_VALUE_EDIT(pDirectonalLightComponent, "LightSize", float, GetLightSize, SetLightSize);

				ShowNodeEnd();
			}
		}
	}
};


#endif