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
    MRenderPass();
    ~MRenderPass();

public:

#if RENDER_GRAPHICS == MORTY_DIRECTX_11

#elif RENDER_GRAPHICS == MORTY_VULKAN
    
    VkRenderPass m_VkRenderPass;
#endif


    class MIRenderTarget* m_pRenderTarget;

    std::vector<MSubpass> m_vSubpass;

};

#endif
