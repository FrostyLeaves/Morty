#pragma once

#include "Component/MModelComponent.h"
#include "Engine/MEngine.h"
#include "MComponentProperty.h"

namespace morty
{

class PropertyMModelComponent : public MComponentProperty
{
public:
    void EditEntity(MainEditor* editor, MEntity* pEntity) override
    {
        MORTY_UNUSED(editor);

        if (auto* pModelComponent = pEntity->GetComponent<MModelComponent>())
        {
            if (m_editProperty.ShowNodeBegin("ModelComponent"))
            {

                PROPERTY_VALUE_GET_SET_EDIT(
                        pModelComponent,
                        "Bounding",
                        bool,
                        GetBoundingBoxVisiable,
                        SetBoundingBoxVisiable
                );

                EditAnimation(pModelComponent);

                m_editProperty.ShowNodeEnd();
            }
        }
    }


    void EditAnimation(MModelComponent* pModelComponent);
};


}// namespace morty