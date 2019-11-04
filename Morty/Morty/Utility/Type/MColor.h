/**
 * @File         MString
 * 
 * @Created      2019-11-04 23:01:32
 *
 * @Author       Morty
**/

#ifndef _M_MCOLOR_H_
#define _M_MCOLOR_H_

#include "Vector.h"

class MColor
{
public:
	MColor();
	MColor(const float& r, const float& g, const float& b, const float& a = 1.0f);
	~MColor();


	Vector3 ToVector3() { return Vector3(r, g, b); }
	Vector4 ToVector4() { return Vector4(r, g, b, a); }


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

};


#endif
