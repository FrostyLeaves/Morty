#include "MColor.h"

MColor::MColor()
	: r(0.0f)
	, g(0.0f)
	, b(0.0f)
	, a(0.0f)
{

}

MColor::MColor(const float& r, const float& g, const float& b, const float& a /*= 1.0f*/)
	: r(r)
	, g(g)
	, b(b)
	, a(a)
{

}

MColor::~MColor()
{

}
