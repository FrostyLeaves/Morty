#include "Utility/MColor.h"

using namespace morty;

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

MColor::MColor(const Vector3& vec3, const float& a /*= 1.0f*/)
	: r(vec3.x)
	, g(vec3.y)
	, b(vec3.z)
	, a(a)
{

}

MColor::MColor(const Vector4& vec4)
	: r(vec4.x)
	, g(vec4.y)
	, b(vec4.z)
	, a(vec4.w)
{

}

MColor::~MColor()
{

}

MColor MColor::Black = MColor(0, 0, 0, 1);
MColor MColor::White = MColor(1, 1, 1, 1);
MColor MColor::Black_T = MColor(0, 0, 0, 0);
MColor MColor::White_T = MColor(1, 1, 1, 0);
MColor MColor::Red = MColor(1, 0, 0, 1);