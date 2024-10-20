#include "MRenderTargetManager.h"
#include "Engine/MEngine.h"
#include "MRenderTaskNode.h"
#include "System/MRenderSystem.h"

using namespace morty;

MORTY_CLASS_IMPLEMENT(MRenderTargetManager, MObject)


MRenderTaskTarget* MRenderTargetManager::CreateRenderTarget(const MStringId& name)
{
    if (auto pResult = FindRenderTarget(name))
    {
        MORTY_ASSERT(false);
        return pResult;
    }

    auto pTarget            = std::make_unique<MRenderTaskTarget>();
    m_renderTaskTable[name] = std::move(pTarget);

    return m_renderTaskTable[name].get();
}

MRenderTaskTarget* MRenderTargetManager::CreateRenderTarget(const MRenderTargetManager::RenderTargetDesc& desc)
{
    return CreateRenderTarget(desc.name)
            ->InitName(desc.name)
            ->InitResizePolicy(desc.resizePolicy, desc.scale)
            ->InitSharedPolicy(desc.sharedPolicy)
            ->InitTextureDesc(desc.textureDesc);
}

MRenderTaskTarget* MRenderTargetManager::CreateRenderTarget(const fbs::MRenderGraphTargetDesc& fbDesc)
{
    return CreateRenderTarget(RenderTargetDesc{
            .name         = MStringId(fbDesc.name()->str()),
            .scale        = fbDesc.scale(),
            .resizePolicy = fbDesc.resize_policy(),
            .sharedPolicy = fbDesc.shared_policy(),
            .textureDesc  = MTexture::CreateTextureFbs(*fbDesc.texture_desc()),
    });
}

MRenderTaskTarget* MRenderTargetManager::FindRenderTarget(const MStringId& name) const
{
    const auto findResult = m_renderTaskTable.find(name);
    if (findResult != m_renderTaskTable.end()) { return findResult->second.get(); }

    return nullptr;
}

MTexturePtr MRenderTargetManager::FindRenderTexture(const MStringId& name) const
{
    if (const auto pResult = FindRenderTarget(name)) { return pResult->GetTexture(); }

    return nullptr;
}

void MRenderTargetManager::ResizeRenderTarget(const Vector2i& size)
{
    const auto pDevice = GetEngine()->FindSystem<MRenderSystem>()->GetDevice();

    for (const auto& pr: m_renderTaskTable)
    {
        if (pr.second->GetResizePolicy() == MEResizePolicy::Scale)
        {
            auto     pTexture = pr.second->GetTexture();

            Vector2i n2TexelFormatSize{};
            n2TexelFormatSize.x =
                    static_cast<float>(size.x) * pr.second->GetScale() + ((size.x % pr.second->GetTexelSize()) != 0);
            n2TexelFormatSize.y =
                    static_cast<float>(size.y) * pr.second->GetScale() + ((size.y % pr.second->GetTexelSize()) != 0);

            pTexture->Resize(pDevice, Vector3i(n2TexelFormatSize.x, n2TexelFormatSize.y, pTexture->GetSize().z));
        }
    }
}

MTextureArray MRenderTargetManager::GetOutputTextures() const
{
    MTextureArray output;

    for (const auto& pr: m_renderTaskTable)
    {
        if (pr.second->GetSharedPolicy() == MESharedPolicy::Exclusive) { output.push_back(pr.second->GetTexture()); }
    }

    return output;
}

flatbuffers::Offset<void>
MRenderTargetManager::SerializeRenderTarget(MRenderTaskTarget* target, flatbuffers::FlatBufferBuilder& builder)
{
    auto fbName        = builder.CreateString(target->GetName().ToString());
    auto fbTextureDesc = MTexture::SerializeFbs(target->GetTexture()->GetTextureDesc(), builder);

    fbs::MRenderGraphTargetDescBuilder targetBuilder(builder);

    targetBuilder.add_name(fbName);
    targetBuilder.add_scale(target->GetScale());
    targetBuilder.add_resize_policy(target->GetResizePolicy());
    targetBuilder.add_shared_policy(target->GetSharedPolicy());
    targetBuilder.add_texture_desc(fbTextureDesc.o);

    return targetBuilder.Finish().Union();
}
