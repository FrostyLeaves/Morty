#ifndef _PROPERTY_MSPOTLIGHT_H_
#define _PROPERTY_MSPOTLIGHT_H_

#include "PropertyBase.h"
#include "Light/MSpotLight.h"

class PropertyMSpotLight : public PropertyBase
{
public:
	virtual void EditObject(MObject* pObject) override
	{
		if (MSpotLight* pNode = pObject->DynamicCast<MSpotLight>())
		{
			if (ShowNodeBegin("Light"))
			{
				PROPERTY_VALUE_EDIT(pNode, "Diffuse", MColor, GetDiffuseColor, SetDiffuseColor);
				PROPERTY_VALUE_EDIT(pNode, "Specular", MColor, GetSpecularColor, SetSpecularColor);
				PROPERTY_VALUE_EDIT_SPEED_MIN_MAX(pNode, "Inner CutOff", float, GetInnerCutOff, SetInnerCutOff, 1.0f, 0.0f, 180.0f);
				PROPERTY_VALUE_EDIT_SPEED_MIN_MAX(pNode, "Outer CutOff", float, GetOuterCutOff, SetOuterCutOff, 1.0f, 0.0f, 180.0f);
	
				ShowNodeEnd();
			}
		}
	}
};


#endif