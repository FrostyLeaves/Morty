#pragma once

#include "Property/PropertyBase.h"

namespace morty
{

class MEntity;
class MainEditor;
class MComponentProperty
{
public:
    virtual ~MComponentProperty()                                 = default;
    virtual void EditEntity(MainEditor* editor, MEntity* pEntity) = 0;

protected:
    PropertyBase m_editProperty;
};

}// namespace morty

#define PROPERTY_NODE_EDIT(NODE, KEY_NAME, TYPE, GET_FUNC, SET_FUNC)                                                   \
    if (m_editProperty.ShowNodeBegin(KEY_NAME))                                                                        \
    {                                                                                                                  \
        TYPE value = NODE->GET_FUNC();                                                                                 \
        if (m_editProperty.Edit##TYPE(value)) { NODE->SET_FUNC(value); }                                               \
        m_editProperty.ShowNodeEnd();                                                                                  \
    }

#define PROPERTY_VALUE_GET_SET_EDIT(NODE, KEY_NAME, TYPE, GET_FUNC, SET_FUNC)                                          \
    m_editProperty.ShowValueBegin(KEY_NAME);                                                                           \
    {                                                                                                                  \
        TYPE value = NODE->GET_FUNC();                                                                                 \
        if (m_editProperty.Edit##TYPE(value)) { NODE->SET_FUNC(value); }                                               \
        m_editProperty.ShowValueEnd();                                                                                 \
    }

#define PROPERTY_VALUE_EDIT_SPEED_MIN_MAX(NODE, KEY_NAME, TYPE, GET_FUNC, SET_FUNC, SPEED, MIN_VAR, MAX_VAR)           \
    m_editProperty.ShowValueBegin(KEY_NAME);                                                                           \
    {                                                                                                                  \
        TYPE value = NODE->GET_FUNC();                                                                                 \
        if (m_editProperty.Edit##TYPE(value, SPEED, MIN_VAR, MAX_VAR)) { NODE->SET_FUNC(value); }                      \
        m_editProperty.ShowValueEnd();                                                                                 \
    }
