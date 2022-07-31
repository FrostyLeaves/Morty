/**
 * @File         MRenderPipeline
 * 
 * @Created      2022-07-23 22:09:42
 *
 * @Author       Pobrecito
**/

#ifndef _M_MRENDERPIPELINE_H_
#define _M_MRENDERPIPELINE_H_
#include "Render/MRenderGlobal.h"


class MMaterial;
class MRenderPass;
class MORTY_API MRenderPipeline
{
public:
    MRenderPipeline();
    virtual ~MRenderPipeline();

public:

private:

    std::shared_ptr<MMaterial> m_pMaterial;
    MRenderPass* m_pRenderPass;
    uint32_t m_nSubPassIdx;


#if RENDER_GRAPHICS == MORTY_VULKAN
    VkPipeline m_vkPipeline;
#endif 
};


#endif
