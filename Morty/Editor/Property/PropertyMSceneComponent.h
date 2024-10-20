#pragma once

#include "Component/MSceneComponent.h"
#include "Property/PropertyBase.h"

namespace morty
{

class PropertyMSceneComponent : public PropertyBase
{
public:
    virtual void EditEntity(MEntity* pEntity) override
    {
        if (MSceneComponent* pSceneComponent = pEntity->GetComponent<MSceneComponent>())
        {
            if (ShowNodeBegin("SceneComponent"))
            {
                PROPERTY_NODE_EDIT(pSceneComponent, "Transform", MTransform, GetTransform, SetTransform);
                PROPERTY_VALUE_GET_SET_EDIT(pSceneComponent, "Visible", bool, GetVisible, SetVisible);
                ShowNodeEnd();
            }
        }
    }
};

}// namespace morty