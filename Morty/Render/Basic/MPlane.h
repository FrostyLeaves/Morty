/**
 * @File         MPlane
 * 
 * @Created      2020-04-14 22:40:14
 *
 * @Author       DoubleYe
 * 
 * http://www.cs.otago.ac.nz/postgrads/alexis/planeExtraction.pdf
**/

#ifndef _M_MPLANE_H_
#define _M_MPLANE_H_
#include "MGlobal.h"
#include "Vector.h"

class MPlane
{
public:
	MPlane();
	~MPlane();

public:

	bool IsOnFront(const Vector3& position);

	float GetDistance(const Vector3& position);

public:
	
	//ax + by + cz + d = 0
	
    Vector4 m_v4Plane;
	
	
};


#endif
