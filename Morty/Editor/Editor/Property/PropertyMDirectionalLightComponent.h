#ifndef _PROPERTY_MDIRECTIONALLIGHT_H_
#define _PROPERTY_MDIRECTIONALLIGHT_H_

#include "PropertyBase.h"
#include "MDirectionalLightComponent.h"



class PropertyMDirectionalLightComponent : public PropertyBase
{
public:
	virtual void EditNode(MNode* pNode) override
	{
		if (MDirectionalLightComponent* pDirectonalLightComponent = pNode->GetComponent<MDirectionalLightComponent>())
		{
			if (ShowNodeBegin("Light"))
			{
				PROPERTY_VALUE_EDIT(pDirectonalLightComponent, "Color", MColor, GetColor, SetColor);
				PROPERTY_VALUE_EDIT(pDirectonalLightComponent, "Intensity", float, GetLightIntensity, SetLightIntensity);

				ShowNodeEnd();
			}
		}
	}
};


#endif