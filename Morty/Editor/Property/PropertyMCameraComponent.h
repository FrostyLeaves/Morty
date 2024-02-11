#pragma once

#include "Property/PropertyBase.h"
#include "Component/MCameraComponent.h"
#include <stdint.h>

MORTY_SPACE_BEGIN

class PropertyMCameraComponent : public PropertyBase
{
public:
	virtual void EditEntity(MEntity* pEntity) override
	{
		if (MCameraComponent* pCameraComponent = pEntity->GetComponent<MCameraComponent>())
		{
			if (ShowNodeBegin("CameraComponent"))
			{
				ShowValueBegin("Type");
				MECameraType eType = pCameraComponent->GetCameraType();
				size_t nSelected = eType == MECameraType::EPerspective ? 0 : 1;
				if (EditEnum({ "Perspective", "Orthographic" }, nSelected))
				{
					pCameraComponent->SetCameraType(nSelected == 0 ? MECameraType::EPerspective : MECameraType::EOrthographic);
				}
				ShowValueEnd();

				if (MECameraType::EPerspective == eType)
				{
					PROPERTY_VALUE_EDIT(pCameraComponent, "Fov", float, GetFov, SetFov);
					PROPERTY_VALUE_EDIT(pCameraComponent, "Near-Far", Vector2, GetZNearFar, SetZNearFar);
				}
				else
				{
					PROPERTY_VALUE_EDIT(pCameraComponent, "Width", float, GetWidth, SetWidth);
					PROPERTY_VALUE_EDIT(pCameraComponent, "Height", float, GetHeight, SetHeight);
				}

				ShowNodeEnd();
			}
		}
	}
};


MORTY_SPACE_END
