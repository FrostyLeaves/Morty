/**
* @File         MRenderStatistics
*
* @Created      2020-03-08 16:54:00
*
* @Author       Morty
**/

#ifndef _M_RENDERSTATISTICS_H_
#define _M_RENDERSTATISTICS_H_
#include "MGlobal.h"
#include "MSingleInstance.h"

#if MORTY_RENDER_DATA_STATISTICS
class MORTY_API MRenderStatistics
{
public:
	static MRenderStatistics* GetInstance();
public:
	uint32_t unTriangleCount;
};

#endif

#endif
