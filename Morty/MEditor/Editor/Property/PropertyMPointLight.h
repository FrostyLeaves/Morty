#ifndef _PROPERTY_MPOINTLIGHT_H_
#define _PROPERTY_MPOINTLIGHT_H_

#include "PropertyBase.h"
#include "MPointLight.h"

class PropertyMPointLight : public PropertyBase
{
public:
	virtual void EditObject(MObject* pObject) override
	{
		if (MPointLight* pNode = pObject->DynamicCast<MPointLight>())
		{
			if (ShowNodeBegin("Light"))
			{
				PROPERTY_VALUE_EDIT(pNode, "Diffuse", MColor, GetDiffuseColor, SetDiffuseColor);
				PROPERTY_VALUE_EDIT(pNode, "Specular", MColor, GetSpecularColor, SetSpecularColor);
				ShowNodeEnd();
			}
		}
	}
};


#endif