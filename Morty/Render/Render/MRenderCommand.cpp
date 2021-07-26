#include "MRenderCommand.h"

MViewportInfo::MViewportInfo()
	: x(0)
	, y(0)
	, width(0)
	, height(0)
	, minz(0)
	, maxz(0)
{

}

MViewportInfo::MViewportInfo(const float& _x, const float& _y, const float& _width, const float& _height)
	: x(_x)
	, y(_y)
	, width(_width)
	, height(_height)
	, minz(0.0f)
	, maxz(1.0f)
{

}

MScissorInfo::MScissorInfo()
	: x(0)
	, y(0)
	, width(0)
	, height(0)
{

}

MScissorInfo::MScissorInfo(const float& _x, const float& _y, const float& _width, const float& _height)
	: x(_x)
	, y(_y)
	, width(_width)
	, height(_height)
{

}

MRenderPassStage::MRenderPassStage()
	: pRenderPass(nullptr)
	, nSubpassIdx(0)
{

}

MRenderPassStage::MRenderPassStage(MRenderPass* p, const uint32_t& n)
	: pRenderPass(p)
	, nSubpassIdx(n)
{

}
