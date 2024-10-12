/**
 * @File         MSkyBoxSystem
 * 
 * @Created      2022-01-23 15:01:01
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"
#include "Engine/MSystem.h"

#include "Component/MComponent.h"

namespace morty
{

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
    void GenerateEnvironmentTexture(MSkyBoxComponent* pComponent);

private:
    void GenerateEnvironmentWork(MSkyBoxComponent* pComponent);
};

}// namespace morty