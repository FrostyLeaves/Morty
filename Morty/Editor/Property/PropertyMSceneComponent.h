#pragma once

#include "Component/MSceneComponent.h"
#include "MComponentProperty.h"

namespace morty
{

class PropertyMSceneComponent : public MComponentProperty
{
public:
    void EditEntity(MainEditor* editor, MEntity* pEntity) override
    {
        MORTY_UNUSED(editor);

        if (auto* pSceneComponent = pEntity->GetComponent<MSceneComponent>())
        {
            if (m_editProperty.ShowNodeBegin("SceneComponent"))
            {
                PROPERTY_NODE_EDIT(pSceneComponent, "Transform", MTransform, GetTransform, SetTransform);
                PROPERTY_VALUE_GET_SET_EDIT(pSceneComponent, "Visible", bool, GetVisible, SetVisible);
                m_editProperty.ShowNodeEnd();
            }
        }
    }
};

}// namespace morty