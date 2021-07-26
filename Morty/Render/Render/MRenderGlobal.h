/**
 * @File         MIDevice
 * 
 * @Created      2021-7-7 14:20:55
 *
 * @Author       DoubleYe
**/

#ifndef _M_MRENDER_INCLUDE_H_
#define _M_MRENDER_INCLUDE_H_
#include "MGlobal.h"

#if RENDER_GRAPHICS == MORTY_VULKAN
#include "MVulkanWrapper.h"
#endif


class MORTY_API MRenderGlobal
{
public:

	static const char* SHADER_SKELETON_ENABLE;
};

#endif
