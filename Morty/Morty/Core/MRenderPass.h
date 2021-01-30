/**
 * @File         MRenderPass
 * 
 * @Created      2020-07-05 19:33:41
 *
 * @Author       DoubleYe
**/

#ifndef _M_MRENDERPASS_H_
#define _M_MRENDERPASS_H_
#include "MGlobal.h"
#include "MIDevice.h"
#include "Type/MColor.h"
#include "MRenderStructure.h"

#include <array>
#include <vector>

#if RENDER_GRAPHICS == MORTY_DIRECTX_11
#elif RENDER_GRAPHICS == MORTY_VULKAN
#include "MVulkanWrapper.h"
#endif

class MIDevice;
class MIRenderTexture;
class MIRenderTarget;
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

public:
	bool bClearWhenRender;
	MColor cClearColor;
};


struct MORTY_API MFrameBuffer
{
    MFrameBuffer();
    std::vector<MIRenderTexture*> vBackTextures;
    MIRenderTexture* pDepthTexture;
    VkFramebuffer vkFrameBuffer;
    VkExtent2D m_vkExtent2D;
};


class MORTY_API MRenderPass
{
public:
    
public:
    MRenderPass();
    ~MRenderPass();


    void GenerateBuffer(MIDevice* pDevice);
    void DestroyBuffer(MIDevice* pDevice);

public:

    void SetRenderPassID(const uint32_t& unID) { m_unRenderPassID = unID; }
    uint32_t GetRenderPassID() const { return m_unRenderPassID; }

    std::vector<MIRenderTexture*> GetBackTexture(const size_t& nFrameIdx);
    MIRenderTexture* GetDepthTexture(const size_t& nFrameIdx);
    MFrameBuffer* GetFrameBuffer(const size_t& nFrameIdx);

	std::vector<MSubpass> m_vSubpass;
	std::vector<MPassTargetDescription> m_vBackDesc;
	std::vector<MFrameBuffer> m_aFrameBuffers;
    MPassTargetDescription m_DepthDesc;

    uint32_t m_unRenderPassID;

#if RENDER_GRAPHICS == MORTY_DIRECTX_11

#elif RENDER_GRAPHICS == MORTY_VULKAN
    
    std::array<VkRenderPass, M_BUFFER_NUM> m_aVkRenderPass;
	std::array<VkCommandBuffer, M_BUFFER_NUM> m_aVkCommandBuffers;

#endif
};

#endif
