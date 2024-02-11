#include "MRenderTargetManager.h"

#include "Engine/MEngine.h"
#include "System/MRenderSystem.h"

MORTY_CLASS_IMPLEMENT(MRenderTargetManager, MObject)


MRenderTaskTarget* MRenderTargetManager::CreateRenderTarget(const MStringId& name)
{
    if (auto pResult = FindRenderTarget(name))
    {
        MORTY_ASSERT(false);
        return pResult;
    }

    auto pTarget = std::make_unique<MRenderTaskTarget>();
    m_tRenderTaskTable[name] = std::move(pTarget);

    return m_tRenderTaskTable[name].get();
}

MRenderTaskTarget* MRenderTargetManager::FindRenderTarget(const MStringId& name) const
{
    const auto findResult = m_tRenderTaskTable.find(name);
    if (findResult != m_tRenderTaskTable.end())
    {
        return findResult->second.get();
    }

    return nullptr;
}

std::shared_ptr<MTexture> MRenderTargetManager::FindRenderTexture(const MStringId& name) const
{
    if (const auto pResult = FindRenderTarget(name))
    {
        return pResult->GetTexture();
    }

    return nullptr;
}

void MRenderTargetManager::ResizeRenderTarget(const Vector2i& size)
{
    const auto pDevice = GetEngine()->FindSystem<MRenderSystem>()->GetDevice();

    for (const auto& pr : m_tRenderTaskTable)
    {
        if (pr.second->GetResizePolicy() == MRenderTaskTarget::ResizePolicy::Scale)
        {
            auto pTexture = pr.second->GetTexture();

            Vector2i n2TexelFormatSize;
            n2TexelFormatSize.x = size.x * pr.second->GetScale() + ((size.x % pr.second->GetTexelSize()) != 0);
            n2TexelFormatSize.y = size.y * pr.second->GetScale() + ((size.y % pr.second->GetTexelSize()) != 0);

            pTexture->DestroyBuffer(pDevice);
            pTexture->SetSize(Vector3i(n2TexelFormatSize.x, n2TexelFormatSize.y, pTexture->GetSize().z));
            pTexture->GenerateBuffer(pDevice);
        }
    }
}

std::vector<std::shared_ptr<MTexture>> MRenderTargetManager::GetOutputTextures() const
{
    std::vector<std::shared_ptr<MTexture>> output;

    for (const auto& pr : m_tRenderTaskTable)
    {
        if (pr.second->GetSharedPolicy() == MRenderTaskTarget::SharedPolicy::Exclusive)
        {
            output.push_back(pr.second->GetTexture());
        }
    }

    return output;
}
