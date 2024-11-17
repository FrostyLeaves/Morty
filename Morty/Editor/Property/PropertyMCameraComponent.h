#pragma once

#include "Component/MCameraComponent.h"
#include "MComponentProperty.h"

namespace morty
{

class PropertyMCameraComponent : public MComponentProperty
{
public:
    void EditEntity(MainEditor* editor, MEntity* pEntity) override
    {
        MORTY_UNUSED(editor);
        
        if (auto* pCameraComponent = pEntity->GetComponent<MCameraComponent>())
        {
            if (m_editProperty.ShowNodeBegin("CameraComponent"))
            {
                m_editProperty.ShowValueBegin("Type");
                MECameraType eType     = pCameraComponent->GetCameraType();
                size_t       nSelected = eType == MECameraType::EPerspective ? 0 : 1;
                if (m_editProperty.EditEnum({"Perspective", "Orthographic"}, nSelected))
                {
                    pCameraComponent->SetCameraType(
                            nSelected == 0 ? MECameraType::EPerspective : MECameraType::EOrthographic
                    );
                }
                m_editProperty.ShowValueEnd();

                if (MECameraType::EPerspective == eType)
                {
                    PROPERTY_VALUE_GET_SET_EDIT(pCameraComponent, "Fov", float, GetFov, SetFov);
                    PROPERTY_VALUE_GET_SET_EDIT(pCameraComponent, "Near-Far", Vector2, GetZNearFar, SetZNearFar);
                }
                else
                {
                    PROPERTY_VALUE_GET_SET_EDIT(pCameraComponent, "Width", float, GetWidth, SetWidth);
                    PROPERTY_VALUE_GET_SET_EDIT(pCameraComponent, "Height", float, GetHeight, SetHeight);
                }

                m_editProperty.ShowNodeEnd();
            }
        }
    }
};


}// namespace morty
