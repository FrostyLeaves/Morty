/**
 * @File         MPlane
 * 
 * @Created      2020-04-14 22:40:14
 *
 * @Author       DoubleYe
 * 
 * http://www.cs.otago.ac.nz/postgrads/alexis/planeExtraction.pdf
**/

#pragma once

#include "Utility/MGlobal.h"
#include "Math/Vector.h"

MORTY_SPACE_BEGIN

class MPlane
{
public:
	MPlane();
	~MPlane();
	
	bool IsOnFront(const Vector3& position) const;
	float GetDistance(const Vector3& position) const;
	void MoveInNormal(const float& distance);

public:
	
	//ax + by + cz + d = 0
    Vector4 m_v4Plane;
};

MORTY_SPACE_END