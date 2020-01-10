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

class MBoundsOBB;
class MORTY_CLASS MBoundsAABB
{
public:
	MBoundsAABB(const std::vector<Vector3>& vPoints);
	MBoundsAABB(const Matrix4& matWorld, const MBoundsOBB& obb);

	void GetPoints(std::vector<Vector3>& vPoints);

	//更新v3min和v3max，以让v3min-v3max的范围包括该Bounds，即取并集
	void UnionMinMax(Vector3& v3min, Vector3& v3max);

public:
	Vector3 m_v3CenterPoint;
	Vector3 m_v3HalfLength;
	Vector3 m_v3MinPoint;
	Vector3 m_v3MaxPoint;
};

class MORTY_CLASS MBoundsOBB
{
public:
	MBoundsOBB() {}
	MBoundsOBB(const Vector3* vPoints, const unsigned int& unArrayLength);
	
	Vector3 ConvertToOBB(const Vector3& v3Pos) const;
	Vector3 ConvertFromOBB(const Vector3& v3Pos) const;

	void SetPoints(const void* vPoints, const unsigned int& unArrayLength, const unsigned int& unOffset, const unsigned int& unDataSize)
	{
		SetPoints((const MByte*)vPoints, unArrayLength, unOffset, unDataSize);
	}

	void SetPoints(const MByte* vPoints, const unsigned int& unArrayLength, const unsigned int& unOffset, const unsigned int& unDataSize);
	

public:
	Vector3 m_v3MinPoint;
	Vector3 m_v3MaxPoint;
	Vector3 m_v3CenterPoint;
	Vector3 m_v3HalfLength;

	Matrix3 m_matEigVectors;
};


#endif
