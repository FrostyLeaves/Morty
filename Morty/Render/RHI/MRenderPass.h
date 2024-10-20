/**
 * @File         MRenderPass
 * 
 * @Created      2020-07-05 19:33:41
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MRenderGlobal.h"

#include "Utility/MColor.h"

namespace morty
{

class MIDevice;
class MTexture;
enum class MDepthCompareType
{
    Never            = 0,
    Less             = 1,
    Equal            = 2,
    Less_Or_Equal    = 3,
    Greater          = 4,
    Not_Equal        = 5,
    Greater_Or_Equal = 6,
    Always           = 7,
};

class MORTY_API MSubpass
{
public:
    MSubpass();

    ~MSubpass();

    MSubpass(const std::vector<uint32_t>& inputs, const std::vector<uint32_t>& outputs);

public:
    std::vector<uint32_t> m_inputIndex;
    std::vector<uint32_t> m_outputIndex;

    uint32_t              m_unViewMask;
    uint32_t              m_unCorrelationMask;
};

class MORTY_API MPassTargetDescription
{
public:
    MPassTargetDescription() = default;

    MPassTargetDescription(const bool bClear, const MColor& cClearColor, const uint32_t& nMipmap = 0);

    bool     bClearWhenRender = true;
    MColor   cClearColor      = MColor::Black_T;
    uint32_t nMipmapLevel     = 0;
};

struct MORTY_API MRenderTarget {
    MRenderTarget() = default;

    MRenderTarget(const MTexturePtr pTexture, const MPassTargetDescription& desc)
        : pTexture(pTexture)
        , desc(desc)
    {}

    MTexturePtr            pTexture = nullptr;
    MPassTargetDescription desc;

#if RENDER_GRAPHICS == MORTY_VULKAN
    VkImageView m_vkImageView = VK_NULL_HANDLE;
#endif
};

class MORTY_API MRenderTargetGroup
{
public:
    std::vector<MRenderTarget> backTargets;
    MRenderTarget              depthTarget;
    MRenderTarget              shadingRate;
};

class MORTY_API MRenderPass
{
public:
public:
    MRenderPass();

    ~MRenderPass();


    void     GenerateBuffer(MIDevice* pDevice);

    void     DestroyBuffer(MIDevice* pDevice);

    void     Resize(MIDevice* pDevice);

    Vector2i GetFrameBufferSize() const;

public:
    void                  SetDepthTestEnable(bool bEnable) { m_depthTestEnable = bEnable; }

    void                  SetDepthWriteEnable(bool bEnable) { m_depthWriteEnable = bEnable; }

    void                  SetDepthCompareType(MDepthCompareType eType) { m_depthCompareOp = eType; }

    void                  SetStencilTestEnable(bool bEnable) { m_stencilTestEnable = bEnable; }

    void                  AddBackTexture(const MRenderTarget& backTexture);

    void                  SetDepthTexture(const MRenderTarget& backTexture);

    void                  AddBackTexture(MTexturePtr pBackTexture, const MPassTargetDescription& desc);

    void                  SetDepthTexture(MTexturePtr pDepthTexture, const MPassTargetDescription& desc);

    void                  SetShadingRateTexture(MTexturePtr& pTexture);

    void                  SetRenderTarget(const MRenderTargetGroup& renderTarget);

    MTexturePtr           GetBackTexture(size_t nIdx) const;

    MTextureArray         GetBackTextures() const;

    MTexturePtr           GetDepthTexture() const;

    MTexturePtr           GetShadingRateTexture() const;

    /* Multi Viewport.
        *
        * @param unNum: size of viewport.
        *
        */
    void                  SetViewportNum(const uint32_t& unNum) { m_unViewNum = unNum; }

    uint32_t              GetViewportNum() const { return m_unViewNum; }

    std::vector<MSubpass> m_subpass;

public:
    uint32_t           m_unViewNum = 1;

    MRenderTargetGroup m_renderTarget;

#if RENDER_GRAPHICS == MORTY_VULKAN
    //vulkan frame buffer.
    VkExtent2D    m_vkExtent2D    = VkExtent2D();
    VkRenderPass  m_vkRenderPass  = VK_NULL_HANDLE;
    VkFramebuffer m_vkFrameBuffer = VK_NULL_HANDLE;
#endif

#if MORTY_DEBUG
    MString m_strDebugName = "";
#endif

    bool              m_depthTestEnable   = true;
    bool              m_depthWriteEnable  = true;
    bool              m_stencilTestEnable = VK_FALSE;
    MDepthCompareType m_depthCompareOp    = MDepthCompareType::Less_Or_Equal;
};

}// namespace morty