#include "RHI/MRenderPass.h"

#include "RHI/Abstract/MIDevice.h"

using namespace morty;

MSubpass::MSubpass()
    : m_inputIndex()
    , m_outputIndex()
    , m_unViewMask(0b11111111)
    , m_unCorrelationMask(0b11111111)
{}

MSubpass::~MSubpass() {}

MSubpass::MSubpass(const std::vector<uint32_t>& inputs, const std::vector<uint32_t>& outputs)
    : m_inputIndex(inputs)
    , m_outputIndex(outputs)
    , m_unViewMask(0b11111111)
    , m_unCorrelationMask(0b11111111)
{}

MRenderPass::MRenderPass()
{

#if RENDER_GRAPHICS == MORTY_VULKAN
    m_vkExtent2D    = VkExtent2D();
    m_vkFrameBuffer = VK_NULL_HANDLE;
    m_vkRenderPass  = VK_NULL_HANDLE;
#endif
}

MRenderPass::~MRenderPass() {}

void MRenderPass::GenerateBuffer(MIDevice* pDevice)
{
    pDevice->GenerateRenderPass(this);
    //	pDevice->GenerateFrameBuffer(this);
}

void MRenderPass::DestroyBuffer(MIDevice* pDevice)
{
    pDevice->DestroyFrameBuffer(this);
    pDevice->DestroyRenderPass(this);
}

void MRenderPass::Resize(MIDevice* pDevice)
{
    pDevice->DestroyFrameBuffer(this);
    //	pDevice->GenerateFrameBuffer(this);
}

Vector2i MRenderPass::GetFrameBufferSize() const
{
#if RENDER_GRAPHICS == MORTY_VULKAN
    return Vector2i(m_vkExtent2D.width, m_vkExtent2D.height);
#endif

    return Vector2i();
}

void MRenderPass::AddBackTexture(const MRenderTarget& backTexture)
{
    MORTY_ASSERT(backTexture.pTexture);

    m_renderTarget.backTargets.push_back(backTexture);
}

void MRenderPass::SetDepthTexture(const MRenderTarget& backTexture) { m_renderTarget.depthTarget = backTexture; }

void MRenderPass::AddBackTexture(MTexturePtr pBackTexture, const MPassTargetDescription& desc)
{
    MRenderTarget backTexture;
    backTexture.pTexture = pBackTexture;
    backTexture.desc     = desc;

    m_renderTarget.backTargets.push_back(backTexture);
}

void MRenderPass::SetDepthTexture(MTexturePtr pDepthTexture, const MPassTargetDescription& desc)
{
    m_renderTarget.depthTarget.pTexture = pDepthTexture;
    m_renderTarget.depthTarget.desc     = desc;
}

void MRenderPass::SetShadingRateTexture(MTexturePtr& pTexture) { m_renderTarget.shadingRate.pTexture = pTexture; }

void MRenderPass::SetRenderTarget(const MRenderTargetGroup& renderTarget) { m_renderTarget = renderTarget; }

MTexturePtr   MRenderPass::GetBackTexture(size_t nIdx) const { return m_renderTarget.backTargets[nIdx].pTexture; }

MTextureArray MRenderPass::GetBackTextures() const
{
    MTextureArray vTextures;

    for (const MRenderTarget& tex: m_renderTarget.backTargets) vTextures.push_back(tex.pTexture);

    return vTextures;
}

MTexturePtr MRenderPass::GetShadingRateTexture() const { return m_renderTarget.shadingRate.pTexture; }

MTexturePtr MRenderPass::GetDepthTexture() const { return m_renderTarget.depthTarget.pTexture; }

MPassTargetDescription::MPassTargetDescription(const bool bClear, const MColor& cColor, const uint32_t& nMipmap)
    : bClearWhenRender(bClear)
    , cClearColor(cColor)
    , nMipmapLevel(nMipmap)
{}
