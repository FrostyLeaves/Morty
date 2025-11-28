#pragma once

#include "Component/MPointLightComponent.h"
#include "MComponentProperty.h"

namespace morty
{

class PropertyMPointLightComponent : public MComponentProperty
{
public:
    void EditEntity(MainEditor* editor, MEntity* pEntity) override
    {
        MORTY_UNUSED(editor);

        if (auto* component = pEntity->GetComponent<MPointLightComponent>())
        {
            if (m_editProperty.ShowNodeBegin("PointLightComponent"))
            {
                PROPERTY_VALUE_GET_SET_EDIT(component, "Color", MColor, GetColor, SetColor);
                PROPERTY_VALUE_GET_SET_EDIT(component, "Intensity", float, GetLightIntensity, SetLightIntensity);
                m_editProperty.ShowNodeEnd();
            }
        }
    }
};

}// namespace morty