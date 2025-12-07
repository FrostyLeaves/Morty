/**
 * @File         MMeshInstanceSystem
 * 
 * @Created      2021-07-21 14:37:08
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MRenderGlobal.h"
#include "Engine/MSystem.h"
#include "Variant/MVariant.h"

namespace morty
{

class MScene;
class MMaterial;
class MRenderPassCmd;

class MORTY_API MMeshInstanceSystem : public MISystem
{
    MORTY_CLASS(MMeshInstanceSystem)


    void DrawMeshInstances(MScene* scene, MRenderPassCmd* command);

    static MVariant CreateMaterialInstanceData(const MMaterial* temp);
};

}// namespace morty