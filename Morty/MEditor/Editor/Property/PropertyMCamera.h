#ifndef _PROPERTY_MCAMERA_H_
#define _PROPERTY_MCAMERA_H_

#include "PropertyBase.h"
#include "MCamera.h"

class PropertyMCamera : public PropertyBase
{
public:
	virtual void EditObject(MObject* pObject) override
	{
		if (MCamera* pNode = pObject->DynamicCast<MCamera>())
		{
			if (ShowNodeBegin("Camera"))
			{
				ShowValueBegin("Type");
				MCamera::MECameraType eType = pNode->GetCameraType();
				int unSelected = eType == MCamera::EPerspective ? 0 : 1;
				if (EditEnum({ "Perspective", "Orthographic" }, unSelected))
				{
					pNode->SetCameraType(unSelected == 0 ? MCamera::EPerspective : MCamera::EOrthographic);
				}
				ShowValueEnd();

				if (MCamera::EPerspective == eType)
				{
					PROPERTY_VALUE_EDIT(pNode, "Fov", float, GetFov, SetFov);
					PROPERTY_VALUE_EDIT(pNode, "Near-Far", Vector2, GetZNearFar, SetZNearFar);
				}
				else
				{
					PROPERTY_VALUE_EDIT(pNode, "Width", float, GetWidth, SetWidth);
					PROPERTY_VALUE_EDIT(pNode, "Height", float, GetHeight, SetHeight);
				}

				ShowNodeEnd();
			}
		}
	}
};


#endif