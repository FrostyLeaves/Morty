/**
 * @File         MEntitySystem
 * 
 * @Created      2021-07-20 14:06:53
 *
 * @Author       Pobrecito
**/

#ifndef _M_MENTITYSYSTEM_H_
#define _M_MENTITYSYSTEM_H_
#include "Utility/MGlobal.h"
#include "Engine/MSystem.h"
#include "Component/MComponent.h"

class MScene;
class MEntity;
class MResource;

class MORTY_API MEntitySystem : public MISystem
{
    MORTY_CLASS(MEntitySystem)
public:
    MEntitySystem();
    virtual ~MEntitySystem();

public:

    void AddChild(MEntity* pParent, MEntity* pChild);

    std::shared_ptr<MResource> PackEntity(MScene* pScene, const std::vector<MEntity*>& vEntity);

    std::vector<MEntity*> LoadEntity(MScene* pScene, std::shared_ptr<MResource> pResource);

    void FindAllComponentRecursively(MEntity* pEntity, const MType* pComponentType, std::vector<MComponentID>& vResult);

private:

};


#endif
