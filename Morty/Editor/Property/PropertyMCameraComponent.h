#ifndef _PROPERTY_CAMERA_COMPONENT_H_
#define _PROPERTY_CAMERA_COMPONENT_H_

#include "Property/PropertyBase.h"
#include "Component/MCameraComponent.h"

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
				MCameraComponent::MECameraType eType = pCameraComponent->GetCameraType();
				int unSelected = eType == MCameraComponent::MECameraType::EPerspective ? 0 : 1;
				if (EditEnum({ "Perspective", "Orthographic" }, unSelected))
				{
					pCameraComponent->SetCameraType(unSelected == 0 ? MCameraComponent::MECameraType::EPerspective : MCameraComponent::MECameraType::EOrthographic);
				}
				ShowValueEnd();

				if (MCameraComponent::MECameraType::EPerspective == eType)
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


#endif