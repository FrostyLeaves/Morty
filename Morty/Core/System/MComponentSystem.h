/**
 * @File         MComponentSystem
 * 
 * @Created      2021-08-03 14:21:17
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"
#include "Component/MComponent.h"
#include "Component/MComponentGroup.h"
#include "Engine/MSystem.h"

namespace morty
{

class MORTY_API MComponentSystem : public MISystem
{
    MORTY_CLASS(MComponentSystem)

public:
    MComponentSystem();

    virtual ~MComponentSystem();

public:
    template<typename TYPE> void RegisterComponent();


    MIComponentGroup*            CreateComponentGroup(const MType* pComponentType);

private:
    std::map<const MType*, std::function<MIComponentGroup*()>> m_componentGroupFactory;
};

template<typename TYPE> void MComponentSystem::RegisterComponent()
{
    if (!MTypeClass::IsType<TYPE, MComponent>()) return;

    m_componentGroupFactory[TYPE::GetClassType()] = []() {
        return new MComponentGroup<TYPE>();
    };
}

}// namespace morty