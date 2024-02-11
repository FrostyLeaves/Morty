/**
 * @File         MString
 * 
 * @Created      2019-11-04 23:01:32
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"
#include "Math/Vector.h"

MORTY_SPACE_BEGIN

class MORTY_API MColor
{
public:
	MColor();
	MColor(const float& r, const float& g, const float& b, const float& a = 1.0f);
	MColor(const Vector3& vec3, const float& a = 1.0f);
	MColor(const Vector4& vec4);
	~MColor();


	Vector3 ToVector3() const { return Vector3(r, g, b); }
	Vector4 ToVector4() const { return Vector4(r, g, b, a); }

public:

	union
	{
		struct 
		{
			float r;
			float g;
			float b;
			float a;
		};

		float m[4];
	};


	static MColor Black;
	static MColor White;
	static MColor Black_T;
	static MColor White_T;
	static MColor Red;
};

MORTY_SPACE_END