/**
 * @File         MBounds
 * 
 * @Created      2019-11-26 12:23:04
 *
 * @Author       Pobrecito
**/

#ifndef _M_MBOUNDS_H_
#define _M_MBOUNDS_H_
#include "MGlobal.h"
#include "MMesh.h"
#include "MVertex.h"
#include <vector>

class MORTY_CLASS MIBounds
{
public:
	MIBounds() {}
	virtual ~MIBounds() {}

public:
};

class MORTY_CLASS MBoundsAABB
{
public:
	MBoundsAABB(const std::vector<Vector3>& vPoints);

public:
	Vector3 m_v3CenterPoint;
	Vector3 m_v3HalfLength;
	Vector3 m_v3MinPoint;
	Vector3 m_v3MaxPoint;
};

class MORTY_CLASS MBoundsOBB
{
public:

	MBoundsOBB(const std::vector<Vector3>& vPoints);
	
	Vector3 ConvertToOBB(const Vector3& v3Pos) const;
	Vector3 ConvertFromOBB(const Vector3& v3Pos) const;

public:
	Vector3 m_v3MinPoint;
	Vector3 m_v3MaxPoint;
	Vector3 m_v3CenterPoint;
	Vector3 m_v3HalfLength;

	Matrix3 m_matEigVectors;
};


#endif
