/**
 * @File         MRenderPass
 * 
 * @Created      2020-07-05 19:33:41
 *
 * @Author       Pobrecito
**/

#ifndef _M_MRENDERPASS_H_
#define _M_MRENDERPASS_H_
#include "MGlobal.h"
#include "MIDevice.h"
#include "Type/MColor.h"

#include <array>
#include <vector>

#if RENDER_GRAPHICS == MORTY_DIRECTX_11
#elif RENDER_GRAPHICS == MORTY_VULKAN
#include "MVulkanWrapper.h"
#endif

class MIRenderTarget;
class MORTY_CLASS MSubpass
{
public:
    MSubpass();
    ~MSubpass();

public:
    MString m_strName;
    std::vector<uint32_t> m_vInputIndex;
    std::vector<uint32_t> m_vOutputIndex;

};

class MORTY_CLASS MPassTargetDescription
{
public:
	MPassTargetDescription();
	MPassTargetDescription(const bool bClear, const MColor& cClearColor);



public:
    MString m_strName;
	bool bClearWhenRender;
	MColor cClearColor;

#if RENDER_GRAPHICS == MORTY_VULKAN
	VkFormat m_vkTargetFormat;
#endif
};

class MORTY_CLASS MRenderPass
{
public:
    
public:
    MRenderPass();
    ~MRenderPass();

public:

    void SetRenderPassID(const uint32_t& unID) { m_unRenderPassID = unID; }
    uint32_t GetRenderPassID() const { return m_unRenderPassID; }

	std::vector<MSubpass> m_vSubpass;

    std::vector<MPassTargetDescription> m_vBackDesc;
    MPassTargetDescription m_DepthDesc;

    uint32_t m_unRenderPassID;

#if RENDER_GRAPHICS == MORTY_DIRECTX_11

#elif RENDER_GRAPHICS == MORTY_VULKAN
    
    std::array<VkRenderPass, M_BUFFER_NUM> m_aVkRenderPass;

#endif
};

#endif
