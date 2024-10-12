/**
 * @File         MEntitySystem
 * 
 * @Created      2021-07-20 14:06:53
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"
#include "Component/MComponent.h"
#include "Engine/MSystem.h"

namespace morty
{

class MScene;
class MEntity;
class MResource;
class MORTY_API MEntitySystem : public MISystem
{
    MORTY_CLASS(MEntitySystem)
public:
    MEntitySystem();

    virtual ~MEntitySystem();

    void                       AddChild(MEntity* pParent, MEntity* pChild);

    std::shared_ptr<MResource> PackEntity(const std::vector<MEntity*>& vEntity);

    std::vector<MEntity*>      LoadEntity(MScene* pScene, std::shared_ptr<MResource> pResource);

    void FindAllComponentRecursively(MEntity* pEntity, const MType* pComponentType, std::vector<MComponentID>& vResult);
};

}// namespace morty