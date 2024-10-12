#include "Component/MRenderMeshComponent.h"

#include "Engine/MEngine.h"
#include "MRenderNotify.h"
#include "Material/MMaterial.h"
#include "Scene/MEntity.h"
#include "Scene/MScene.h"
#include "Utility/MFunction.h"

#include "Resource/MMaterialResource.h"
#include "Resource/MMeshResource.h"


#include "Component/MModelComponent.h"
#include "Component/MSceneComponent.h"

#include "System/MRenderSystem.h"
#include "System/MResourceSystem.h"

#include "Flatbuffer/MRenderMeshComponent_generated.h"

using namespace morty;

MORTY_CLASS_IMPLEMENT(MRenderMeshComponent, MComponent)

MRenderMeshComponent::MRenderMeshComponent()
    : MComponent()
    , m_shadowType(MEShadowType::ENone)
    , m_unDetailLevel(MRenderGlobal::MESH_LOD_LEVEL_RANGE)
{}

MRenderMeshComponent::~MRenderMeshComponent() {}

void MRenderMeshComponent::Release() { Super::Release(); }

void MRenderMeshComponent::SetMaterial(std::shared_ptr<MMaterialResource> pMaterial)
{
    if (m_Material.GetResource() == pMaterial) return;

    m_Material = pMaterial;
    SendComponentNotify(MRenderNotify::NOTIFY_MATERIAL_CHANGED);
}

std::shared_ptr<MMaterialResource> MRenderMeshComponent::GetMaterialResource() const
{
    return m_Material.GetResource<MMaterialResource>();
}

std::shared_ptr<MMaterial> MRenderMeshComponent::GetMaterial() { return m_Material.GetResource<MMaterial>(); }

bool                       MRenderMeshComponent::SetMaterialPath(const MString& strPath)
{
    MResourceSystem* pResourceSystem = GetEngine()->FindSystem<MResourceSystem>();
    if (std::shared_ptr<MResource> pResource = pResourceSystem->LoadResource(strPath))
    {
        if (std::shared_ptr<MMaterialResource> pMaterialResource =
                    MTypeClass::DynamicCast<MMaterialResource>(pResource))
        {
            SetMaterial(pMaterialResource);
            return true;
        }
    }

    return false;
}

void MRenderMeshComponent::Load(std::shared_ptr<MResource> pResource)
{
    if (!pResource) return;

    if (std::shared_ptr<MMeshResource> pMeshResource = MTypeClass::DynamicCast<MMeshResource>(pResource))
    {
        m_Mesh.SetResource(pResource);
        SendComponentNotify(MRenderNotify::NOTIFY_MESH_CHANGED);
    }
}

void MRenderMeshComponent::SetMeshResourcePath(const MString& strResourcePath)
{
    MResourceSystem*           pResourceSystem = GetEngine()->FindSystem<MResourceSystem>();
    std::shared_ptr<MResource> pResource       = pResourceSystem->LoadResource(strResourcePath);
    Load(pResource);
}

MIMesh* MRenderMeshComponent::GetMesh()
{
    std::shared_ptr<MMeshResource> pMeshResource = m_Mesh.GetResource<MMeshResource>();
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
    auto                             fb_material = m_Material.Serialize(fbb).o;
    auto                             fb_mesh     = m_Mesh.Serialize(fbb).o;
    fbs::MRenderMeshComponentBuilder builder(fbb);

    builder.add_gen_dir_shadow(GetGenerateDirLightShadow());
    builder.add_lod((int) GetDetailLevel());
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
    auto                             pResourceSystem = GetEngine()->FindSystem<MResourceSystem>();

    const fbs::MRenderMeshComponent* pComponent = reinterpret_cast<const fbs::MRenderMeshComponent*>(pBufferPointer);

    Super::Deserialize(pComponent->super());

    SetGenerateDirLightShadow(pComponent->gen_dir_shadow());
    SetDetailLevel(pComponent->lod());

    MResourceRef material;
    material.Deserialize(pResourceSystem, pComponent->material());
    SetMaterial(material.GetResource<MMaterialResource>());

    MResourceRef mesh;
    mesh.Deserialize(pResourceSystem, pComponent->mesh());
    Load(mesh.GetResource());
}
