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

#include <vector>

#if RENDER_GRAPHICS == MORTY_DIRECTX_11
#elif RENDER_GRAPHICS == MORTY_VULKAN
#include <vulkan/vulkan.h>
#endif

class MIRenderTarget;
class MORTY_CLASS MSubpass
{
public:
    MSubpass();
    ~MSubpass();

public:

};

class MORTY_CLASS MRenderPass
{
public:
    struct MRTDesc
    {
        bool bClearWhenRender = true;
    };
public:
    MRenderPass(MIRenderTarget* pRenderTarget);
    ~MRenderPass();

public:

    void SetRenderPassID(const uint32_t& unID) { m_unRenderPassID = unID; }
    uint32_t GetRenderPassID() const { return m_unRenderPassID; }

	MIRenderTarget* m_pRenderTarget;
	std::vector<MSubpass> m_vSubpass;

    std::vector<MRTDesc> m_vBackDesc;
    MRTDesc m_DepthDesc;

    uint32_t m_unRenderPassID;

#if RENDER_GRAPHICS == MORTY_DIRECTX_11

#elif RENDER_GRAPHICS == MORTY_VULKAN
    
    VkRenderPass m_VkRenderPass;
#endif
};

#endif
