#pragma once

#include "Component/MSpotLightComponent.h"
#include "MComponentProperty.h"

namespace morty
{

class PropertyMSpotLightComponent : public MComponentProperty
{
public:
    void EditEntity(MainEditor* editor, MEntity* pEntity) override
    {
        MORTY_UNUSED(editor);

        if (auto* pComponent = pEntity->GetComponent<MSpotLightComponent>())
        {
            if (m_editProperty.ShowNodeBegin("Light"))
            {
                PROPERTY_VALUE_GET_SET_EDIT(pComponent, "Color", MColor, GetColor, SetColor);
                PROPERTY_VALUE_GET_SET_EDIT(pComponent, "Intensity", float, GetLightIntensity, SetLightIntensity);
                PROPERTY_VALUE_EDIT_SPEED_MIN_MAX(
                        pComponent,
                        "Inner CutOff",
                        float,
                        GetInnerCutOff,
                        SetInnerCutOff,
                        1.0f,
                        0.0f,
                        180.0f
                );
                PROPERTY_VALUE_EDIT_SPEED_MIN_MAX(
                        pComponent,
                        "Outer CutOff",
                        float,
                        GetOuterCutOff,
                        SetOuterCutOff,
                        1.0f,
                        0.0f,
                        180.0f
                );

                m_editProperty.ShowNodeEnd();
            }
        }
    }
};

}// namespace morty