#include "System/MMeshInstanceSystem.h"
#include "Batch/Mesh/MMeshInstanceManager.h"
#include "Component/MSceneComponent.h"
#include "Material/MMaterial.h"
#include "Material/MMaterialTemplate.h"
#include "Mesh/MMeshManager.h"
#include "Model/MSkeletalAnimation.h"
#include "RHI/Command/MRenderPassCmd.h"
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
    auto instancingName = shaderProgram->GetPropertyBlock().GetInstancingName(MInstanceDataType::Property);
    auto parameterSet   = mat->GetMaterialParameterSet();
    if (parameterSet == nullptr) return {};
    auto storageParam = parameterSet->FindStorageParam(instancingName);
    if (storageParam == nullptr) return {};

    return MVariant::Clone(storageParam->var);
}

void MMeshInstanceSystem::DrawMeshInstances(MScene* scene, MRenderPassCmd* command)
{
    if (scene == nullptr) return;
    if (command == nullptr) return;

    auto meshManager         = scene->GetManager<MMeshManager>();
    auto meshInstanceManager = scene->GetManager<MMeshInstanceManager>();
    if (meshManager == nullptr) return;
    if (meshInstanceManager == nullptr) return;

    //auto  vertexBuffer = meshManager->GetVertexBuffer();
    //auto  indexBuffer  = meshManager->GetIndexBuffer();

    auto& batchGroups = meshInstanceManager->GetBatchGroups();

    // Iterate through all mesh instances and issue draw calls
    for (const auto& group: batchGroups)
    {
        auto parameterSet = group->GetParameterSet();
        command->SetShaderParameterSet(parameterSet);
        //        command->DrawIndexedIndirect(vertexBuffer, indexBuffer,);
    }
}