#pragma once

#include "Component/MDirectionalLightComponent.h"
#include "Property/PropertyBase.h"

namespace morty
{

class PropertyMDirectionalLightComponent : public PropertyBase
{
public:
    virtual void EditEntity(MEntity* pEntity) override
    {
        if (MDirectionalLightComponent* pDirectonalLightComponent = pEntity->GetComponent<MDirectionalLightComponent>())
        {
            if (ShowNodeBegin("DirLightComponent"))
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

                ShowNodeEnd();
            }
        }
    }
};


}// namespace morty