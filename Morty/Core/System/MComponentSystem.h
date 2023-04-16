/**
 * @File         MComponentSystem
 * 
 * @Created      2021-08-03 14:21:17
 *
 * @Author       DoubleYe
**/

#ifndef _M_MCOMPONENTSYSTEM_H_
#define _M_MCOMPONENTSYSTEM_H_
#include "Utility/MGlobal.h"
#include "Engine/MSystem.h"
#include "Component/MComponent.h"

class MORTY_API MComponentSystem : public MISystem
{
    MORTY_CLASS(MComponentSystem)

public:
    MComponentSystem();
    virtual ~MComponentSystem();

public:
    
    template<typename TYPE>
    void RegisterComponent();


    MIComponentGroup* CreateComponentGroup(const MType* pComponentType);

private:

    std::map<const MType*, std::function<MIComponentGroup*()>> m_tComponentGroupFactory;
};

template<typename TYPE>
void MComponentSystem::RegisterComponent()
{
    if (!MTypeClass::IsType<TYPE, MComponent>())
        return;

    m_tComponentGroupFactory[TYPE::GetClassType()] = []() { return new MComponentGroup<TYPE>(); };
}

#endif
