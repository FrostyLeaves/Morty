#include "System/MMeshInstanceSystem.h"
#include "Component/MSceneComponent.h"
#include "Material/MMaterial.h"
#include "Material/MMaterialTemplate.h"
#include "Model/MSkeletalAnimation.h"
#include "Scene/MScene.h"

using namespace morty;

MORTY_CLASS_IMPLEMENT(MMeshInstanceSystem, MISystem)

MVariant MMeshInstanceSystem::CreateMaterialInstanceData(const MMaterial* mat)
{
    if (mat == nullptr) return {};
    auto temp = mat->GetTemplate();
    if (temp->GetDefaultPass() == nullptr) return {};
    auto shaderProgram = temp->GetDefaultPass()->GetShaderProgram();
    if (shaderProgram == nullptr) return {};
    auto instancingName = shaderProgram->GetPropertyBlock().GetInstancingName();
    auto parameterSet   = mat->GetMaterialParameterSet();
    if (parameterSet == nullptr) return {};
    auto storageParam = parameterSet->FindStorageParam(instancingName);
    if (storageParam == nullptr) return {};

    return MVariant::Clone(storageParam->var);
}