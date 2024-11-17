#pragma once

#include "Component/MDirectionalLightComponent.h"
#include "MComponentProperty.h"

namespace morty
{

class PropertyMDirectionalLightComponent : public MComponentProperty
{
public:
    void EditEntity(MainEditor* editor, MEntity* pEntity) override
    {
        MORTY_UNUSED(editor);

        if (auto* pDirectonalLightComponent = pEntity->GetComponent<MDirectionalLightComponent>())
        {
            if (m_editProperty.ShowNodeBegin("DirLightComponent"))
            {
                PROPERTY_VALUE_GET_SET_EDIT(pDirectonalLightComponent, "Enable", bool, GetLightEnable, SetLightEnable);
                PROPERTY_VALUE_GET_SET_EDIT(pDirectonalLightComponent, "Color", MColor, GetColor, SetColor);
                PROPERTY_VALUE_GET_SET_EDIT(
                        pDirectonalLightComponent,
                        "Intensity",
                        float,
                        GetLightIntensity,
                        SetLightIntensity
                );
                PROPERTY_VALUE_EDIT_SPEED_MIN_MAX(
                        pDirectonalLightComponent,
                        "LightSize",
                        float,
                        GetLightSize,
                        SetLightSize,
                        0.01f,
                        0.0f,
                        1000.0f
                );

                m_editProperty.ShowNodeEnd();
            }
        }
    }
};


}// namespace morty