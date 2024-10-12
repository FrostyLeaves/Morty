#pragma once

#include "Component/MPointLightComponent.h"
#include "Property/PropertyBase.h"

namespace morty
{

class PropertyMPointLightComponent : public PropertyBase
{
public:
    virtual void EditEntity(MEntity* pEntity) override
    {
        if (MPointLightComponent* pComponent =
                    pEntity->GetComponent<MPointLightComponent>())
        {
            if (ShowNodeBegin("PointLightComponent"))
            {
                PROPERTY_VALUE_EDIT(pComponent, "Color", MColor, GetColor, SetColor);
                PROPERTY_VALUE_EDIT(
                        pComponent,
                        "Intensity",
                        float,
                        GetLightIntensity,
                        SetLightIntensity
                );
                ShowNodeEnd();
            }
        }
    }
};

}// namespace morty