/**
 * @File         MRenderPass
 * 
 * @Created      2020-07-05 19:33:41
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Render/MRenderGlobal.h"

#include "Utility/MColor.h"

class MIDevice;
class MTexture;


enum class MDepthCompareType
{
    Never = 0,
    Less = 1,
    Equal = 2,
    Less_Or_Equal = 3,
    Greater = 4,
    Not_Equal = 5,
    Greater_Or_Equal = 6,
    Always = 7,
};

class MORTY_API MSubpass
{
public:
    MSubpass();
    ~MSubpass();

    MSubpass(const std::vector<uint32_t>& inputs, const std::vector<uint32_t>& outputs);

public:
    std::vector<uint32_t> m_vInputIndex;
    std::vector<uint32_t> m_vOutputIndex;

    uint32_t m_unViewMask;
    uint32_t m_unCorrelationMask;

};

class MORTY_API MPassTargetDescription
{
public:

	MPassTargetDescription() = default;
	MPassTargetDescription(const bool bClear , const MColor& cClearColor, const uint32_t& nMipmap = 0);

	bool bClearWhenRender = true;
	MColor cClearColor = MColor::Black_T;
    uint32_t nMipmapLevel = 0;
};

struct MORTY_API MRenderTarget
{
    MRenderTarget() = default;
    MRenderTarget(const std::shared_ptr<MTexture> pTexture, const MPassTargetDescription& desc)
    : pTexture(pTexture)
    , desc(desc){}

    std::shared_ptr<MTexture> pTexture = nullptr;
    MPassTargetDescription desc;

#if RENDER_GRAPHICS == MORTY_VULKAN
    VkImageView m_VkImageView = VK_NULL_HANDLE;
#endif
};

class MORTY_API MRenderTargetGroup
{
public:
    std::vector<MRenderTarget> backTargets;
    MRenderTarget depthTarget;
    MRenderTarget shadingRate;
};

class MORTY_API MRenderPass
{
public:
    
public:
    MRenderPass();
    ~MRenderPass();


    void GenerateBuffer(MIDevice* pDevice);
    void DestroyBuffer(MIDevice* pDevice);

    void Resize(MIDevice* pDevice);

    Vector2i GetFrameBufferSize() const;

public:

    void SetDepthTestEnable(bool bEnable) { m_bDepthTestEnable = bEnable; }
    void SetDepthWriteEnable(bool bEnable) { m_bDepthWriteEnable = bEnable; }
    void SetDepthCompareType(MDepthCompareType eType) { m_eDepthCompareOp = eType; }
    void SetStencilTestEnable(bool bEnable) { m_bStencilTestEnable = bEnable; }

    void AddBackTexture(const MRenderTarget& backTexture);
    void SetDepthTexture(const MRenderTarget& backTexture);
    void AddBackTexture(std::shared_ptr<MTexture> pBackTexture, const MPassTargetDescription& desc);
    void SetDepthTexture(std::shared_ptr<MTexture> pDepthTexture, const MPassTargetDescription& desc);

    void SetShadingRateTexture(std::shared_ptr<MTexture>& pTexture);

    void SetRenderTarget(const MRenderTargetGroup& renderTarget);

    std::shared_ptr<MTexture> GetBackTexture(size_t nIdx) const;
    std::vector<std::shared_ptr<MTexture>> GetBackTextures() const;
    std::shared_ptr<MTexture> GetDepthTexture() const;
    std::shared_ptr<MTexture> GetShadingRateTexture() const;

    void SetRenderPassID(const uint32_t& unID) { m_unRenderPassID = unID; }
    uint32_t GetRenderPassID() const { return m_unRenderPassID; }


    /* Multi Viewport.
    * 
    * @param unNum: size of viewport.
    * 
    */
    void SetViewportNum(const uint32_t& unNum) { m_unViewNum = unNum; }
    uint32_t GetViewportNum() const { return m_unViewNum; }

	std::vector<MSubpass> m_vSubpass;

    uint32_t m_unRenderPassID = MGlobal::M_INVALID_INDEX;

public:

    uint32_t m_unViewNum = 1;

    MRenderTargetGroup m_renderTarget;

#if RENDER_GRAPHICS == MORTY_VULKAN
    //vulkan frame buffer.
	VkExtent2D m_vkExtent2D = VkExtent2D();
    VkRenderPass m_VkRenderPass = VK_NULL_HANDLE;
    VkFramebuffer m_VkFrameBuffer = VK_NULL_HANDLE;
#endif

#if MORTY_DEBUG
    MString m_strDebugName = "";
#endif

    bool m_bDepthTestEnable = true;
	bool m_bDepthWriteEnable = true;
	bool m_bStencilTestEnable = VK_FALSE;
	MDepthCompareType m_eDepthCompareOp = MDepthCompareType::Less_Or_Equal;

};
