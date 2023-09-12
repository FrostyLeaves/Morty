/**
 * @File         MBounds
 * 
 * @Created      2019-11-26 12:23:04
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"
#include "Math/Vector.h"
#include "Utility/MSerializer.h"

#include <vector>

class MORTY_API MIBounds : public MSerializer
{
public:
	MIBounds() {}
	virtual ~MIBounds() {}

public:

};

class MBoundsOBB;
class MORTY_API MBoundsAABB : public MIBounds
{
public:
	MBoundsAABB() {}
	MBoundsAABB(const std::vector<Vector3>& vPoints);
	MBoundsAABB(const MBoundsAABB& aabb, const Matrix4& matWorld);
	MBoundsAABB(const std::vector<Vector3>& vPoints, const Matrix4& matWorld);
	MBoundsAABB(const Vector3& center, const Vector3& halfLength);

	void SetMinMax(const Vector3& v3Min, const Vector3& v3Max);
	void SetPoints(const std::vector<Vector3>& vPoints);
	void SetBoundsOBB(const Vector3& v3Origin, const Matrix4& matWorld, const MBoundsOBB& obb);

	void GetPoints(std::vector<Vector3>& vPoints) const;

	//����v3min��v3max������v3min-v3max�ķ�Χ������Bounds����ȡ����
	void UnionMinMax(Vector3& v3min, Vector3& v3max) const;

	MBoundsAABB IntersectAABB(const MBoundsAABB& aabb) const;
	bool IsIntersect(const MBoundsAABB& aabb) const;

public:

	virtual flatbuffers::Offset<void> Serialize(flatbuffers::FlatBufferBuilder& fbb) const;
	virtual void Deserialize(const void* pBufferPointer);

public:
	Vector3 m_v3CenterPoint;
	Vector3 m_v3HalfLength;
	Vector3 m_v3MinPoint;
	Vector3 m_v3MaxPoint;
};

class MORTY_API MBoundsOBB : public MIBounds
{
public:
	MBoundsOBB() {}
	MBoundsOBB(const Vector3* vPoints, const uint32_t& unArrayLength);
	
	Vector3 ConvertToOBB(const Vector3& v3Pos) const;
	Vector3 ConvertFromOBB(const Vector3& v3Pos) const;

	void SetPoints(const void* vPoints, const uint32_t& unArrayLength, const uint32_t& unOffset, const uint32_t& unDataSize)
	{
		SetPoints((const MByte*)vPoints, unArrayLength, unOffset, unDataSize);
	}

	void SetPoints(const MByte* vPoints, const uint32_t& unArrayLength, const uint32_t& unOffset, const uint32_t& unDataSize);
	
public:

	virtual flatbuffers::Offset<void> Serialize(flatbuffers::FlatBufferBuilder& fbb) const;
	virtual void Deserialize(const void* pBufferPointer);

public:
	Vector3 m_v3MinPoint;
	Vector3 m_v3MaxPoint;
	Vector3 m_v3CenterPoint;
	Vector3 m_v3HalfLength;

	Matrix3 m_matEigVectors;
};

class MORTY_API MBoundsSphere : public MIBounds
{
public:
	MBoundsSphere();
	MBoundsSphere(const Vector3& v3CenterPoint, const float& fRadius);

	void SetPoints(const void* vPoints, const uint32_t& unArrayLength, const uint32_t& unOffset, const uint32_t& unDataSize)
	{
		SetPoints((const MByte*)vPoints, unArrayLength, unOffset, unDataSize);
	}

	void SetPoints(const MByte* vPoints, const uint32_t& unArrayLength, const uint32_t& unOffset, const uint32_t& unDataSize);

	void SetPoints(const std::vector<Vector3>& vPoints);

	void AddPoint(const Vector3& pos);

	bool IsContain(const Vector3& pos);

public:

	virtual flatbuffers::Offset<void> Serialize(flatbuffers::FlatBufferBuilder& fbb) const;
	virtual void Deserialize(const void* pBufferPointer);

public:

	Vector3 m_v3CenterPoint;
	float m_fRadius;
};
