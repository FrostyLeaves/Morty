/**
 * @File         MRenderPass
 * 
 * @Created      2020-07-05 19:33:41
 *
 * @Author       DoubleYe
**/

#ifndef _M_MRENDERPASS_H_
#define _M_MRENDERPASS_H_
#include "MRenderGlobal.h"

#include "MColor.h"

class MIDevice;
class MTexture;
class MORTY_API MSubpass
{
public:
    MSubpass();
    ~MSubpass();

public:
    std::vector<uint32_t> m_vInputIndex;
    std::vector<uint32_t> m_vOutputIndex;

};

class MORTY_API MPassTargetDescription
{
public:

	MPassTargetDescription();
	MPassTargetDescription(const bool bClear, const MColor& cClearColor);
	MPassTargetDescription(const bool bClear, const bool bAlready, const MColor& cClearColor);

public:
	bool bClearWhenRender;
    bool bAlreadyRender;
	MColor cClearColor;
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

    Vector2 GetFrameBufferSize();

public:

    void SetRenderPassID(const uint32_t& unID) { m_unRenderPassID = unID; }
    uint32_t GetRenderPassID() const { return m_unRenderPassID; }

    std::vector<MTexture*> GetBackTexture();
    MTexture* GetDepthTexture();

	std::vector<MSubpass> m_vSubpass;

    uint32_t m_unRenderPassID;

public:

    //render back to texture
	std::vector<MTexture*> m_vBackTextures;
	std::vector<MPassTargetDescription> m_vBackDesc;

    //render depth to texture
	MTexture* m_pDepthTexture;
	MPassTargetDescription m_DepthDesc;

#if RENDER_GRAPHICS == MORTY_VULKAN
    //vulkan frame buffer.
    VkFramebuffer m_VkFrameBuffer;
	VkExtent2D m_vkExtent2D;

    //vulkan renderpass
    VkRenderPass m_VkRenderPass;
#endif
};

#endif
