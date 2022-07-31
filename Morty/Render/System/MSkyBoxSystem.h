/**
 * @File         MSkyBoxSystem
 * 
 * @Created      2022-01-23 15:01:01
 *
 * @Author       Pobrecito
**/

#ifndef _M_MSKYBOXSYSTEM_H_
#define _M_MSKYBOXSYSTEM_H_
#include "Utility/MGlobal.h"
#include "Engine/MSystem.h"

#include "Component/MComponent.h"

class MIRenderCommand;
class MSkyBoxComponent;
class MORTY_API MSkyBoxSystem : public MISystem
{
public:
    MORTY_CLASS(MSkyBoxSystem)
public:
    MSkyBoxSystem();
    virtual ~MSkyBoxSystem();

public:

    virtual void EngineTick(const float& fDelta);

public:

    void GenerateEnvironmentTexture(MSkyBoxComponent* pComponent);

private:



    std::queue<MSkyBoxComponent*> m_vGenerateQueue;
};


#endif
