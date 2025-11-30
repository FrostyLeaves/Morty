#include "Component/MRenderMeshComponent.h"

#include "Component/MModelComponent.h"
#include "Component/MSceneComponent.h"
#include "Engine/MEngine.h"
#include "MRenderNotify.h"
#include "Material/MMaterial.h"
#include "Resource/MMaterialResource.h"
#include "Resource/MMeshResource.h"
#include "Scene/MEntity.h"
#include "Scene/MScene.h"
#include "System/MMeshInstanceSystem.h"
#include "System/MRenderSystem.h"
#include "System/MResourceSystem.h"
#include "Utility/MFunction.h"
#include "Flatbuffer/MRenderMeshComponent_generated.h"

using namespace morty;

MORTY_CLASS_IMPLEMENT(MRenderMeshComponent, MComponent)

MRenderMeshComponent::MRenderMeshComponent()
    : MComponent()
    , m_shadowType(MEShadowType::ENone)
{}

void MRenderMeshComponent::Release() { Super::Release(); }

void MRenderMeshComponent::SetMaterial(const std::shared_ptr<MMaterialResource>& material)
{
    if (m_material.GetResource() == material) return;

    m_material       = material;
    m_instancingData = MMeshInstanceSystem::CreateMaterialInstanceData(material.get());

    SendComponentNotify(MRenderNotify::NOTIFY_MATERIAL_CHANGED);
}

std::shared_ptr<MMaterialResource> MRenderMeshComponent::GetMaterial() const
{
    return m_material.GetResource<MMaterialResource>();
}

void MRenderMeshComponent::SetMesh(const std::shared_ptr<MMeshResource>& mesh)
{
    if (!mesh) return;

    m_mesh.SetResource(mesh);
    SendComponentNotify(MRenderNotify::NOTIFY_MESH_CHANGED);
}

std::shared_ptr<MMeshResource> MRenderMeshComponent::GetMesh() const { return m_mesh.GetResource<MMeshResource>(); }

void     MRenderMeshComponent::SetInstancingData(const MVariant& value) { m_instancingData = value; }
MVariant MRenderMeshComponent::GetInstancingData() const { return m_instancingData; }

MIMesh*  MRenderMeshComponent::GetDrawMesh()
{
    std::shared_ptr<MMeshResource> pMeshResource = m_mesh.GetResource<MMeshResource>();
    if (!pMeshResource) return nullptr;

    return pMeshResource->GetMesh();
}

void MRenderMeshComponent::SetGenerateDirLightShadow(const bool& bGenerate)
{
    if (m_generateDirLightShadow != bGenerate)
    {
        m_generateDirLightShadow = bGenerate;
        SendComponentNotify(MRenderNotify::NOTIFY_GENERATE_SHADOW_CHANGED);
    }
}

void MRenderMeshComponent::SetSceneCullEnable(bool bEnable) { m_sceneCullEnable = bEnable; }

void MRenderMeshComponent::SetAttachedModelComponentID(MComponentID idx)
{
    if (m_modelComponent == idx) { return; }

    m_modelComponent = idx;
    SendComponentNotify(MRenderNotify::NOTIFY_ATTACHED_SKELETON_CHANGED);
}

flatbuffers::Offset<void> MRenderMeshComponent::Serialize(flatbuffers::FlatBufferBuilder& fbb)
{
    auto                             fb_super    = Super::Serialize(fbb).o;
    auto                             fb_material = m_material.Serialize(fbb).o;
    auto                             fb_mesh     = m_mesh.Serialize(fbb).o;
    fbs::MRenderMeshComponentBuilder builder(fbb);

    builder.add_gen_dir_shadow(GetGenerateDirLightShadow());
    builder.add_material(fb_material);
    builder.add_mesh(fb_mesh);
    builder.add_super(fb_super);


    return builder.Finish().Union();
}

void MRenderMeshComponent::Deserialize(flatbuffers::FlatBufferBuilder& fbb)
{
    const fbs::MRenderMeshComponent* fbcomponent = fbs::GetMRenderMeshComponent(fbb.GetCurrentBufferPointer());
    Deserialize(fbcomponent);
}

void MRenderMeshComponent::Deserialize(const void* pBufferPointer)
{
    auto resourceSystem = GetEngine()->FindSystem<MResourceSystem>();
    auto component      = reinterpret_cast<const fbs::MRenderMeshComponent*>(pBufferPointer);

    Super::Deserialize(component->super());

    SetGenerateDirLightShadow(component->gen_dir_shadow());

    MResourceRef material;
    material.Deserialize(resourceSystem, component->material());
    SetMaterial(material.GetResource<MMaterialResource>());

    MResourceRef mesh;
    mesh.Deserialize(resourceSystem, component->mesh());
    SetMesh(mesh.GetResource<MMeshResource>());
}
