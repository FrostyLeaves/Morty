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

namespace morty
{

class MORTY_API MIBounds
{
public:
    MIBounds() {}

    virtual ~MIBounds() {}

public:
};

class MBoundsOBB;
class MBoundsSphere;
class MORTY_API MBoundsAABB : public MIBounds
{
public:
    MBoundsAABB() {}

    MBoundsAABB(const std::vector<Vector3>& vPoints);

    MBoundsAABB(const MBoundsAABB& aabb, const Matrix4& matWorld);

    MBoundsAABB(const std::vector<Vector3>& vPoints, const Matrix4& matWorld);

    MBoundsAABB(const Vector3& min, const Vector3& max);

    void          SetMinMax(const Vector3& v3Min, const Vector3& v3Max);

    void          SethalfLength(const Vector3& f3HalfLength);

    void          SetPoints(const std::vector<Vector3>& vPoints);

    void          SetBoundsOBB(const Vector3& v3Origin, const Matrix4& matWorld, const MBoundsOBB& obb);

    void          GetPoints(std::vector<Vector3>& vPoints) const;

    void          UnionMinMax(Vector3& v3min, Vector3& v3max) const;

    MBoundsAABB   IntersectAABB(const MBoundsAABB& aabb) const;

    bool          IsIntersect(const MBoundsAABB& aabb) const;

    MBoundsSphere ToSphere() const;

public:
    virtual flatbuffers::Offset<void> Serialize(flatbuffers::FlatBufferBuilder& fbb) const;

    virtual void                      Deserialize(const void* pBufferPointer);

public:
    Vector3 m_centerPoint;
    Vector3 m_halfLength;
    Vector3 m_minPoint;
    Vector3 m_maxPoint;
};

class MORTY_API MBoundsOBB : public MIBounds
{
public:
    MBoundsOBB() {}

    MBoundsOBB(const Vector3* vPoints, const uint32_t& unArrayLength);

    Vector3 ConvertToOBB(const Vector3& v3Pos) const;

    Vector3 ConvertFromOBB(const Vector3& v3Pos) const;

    void
    SetPoints(const void* vPoints, const uint32_t& unArrayLength, const uint32_t& unOffset, const uint32_t& unDataSize)
    {
        SetPoints((const MByte*) vPoints, unArrayLength, unOffset, unDataSize);
    }

    void SetPoints(
            const MByte*    vPoints,
            const uint32_t& unArrayLength,
            const uint32_t& unOffset,
            const uint32_t& unDataSize
    );

public:
    virtual flatbuffers::Offset<void> Serialize(flatbuffers::FlatBufferBuilder& fbb) const;

    virtual void                      Deserialize(const void* pBufferPointer);

public:
    Vector3 m_minPoint;
    Vector3 m_maxPoint;
    Vector3 m_centerPoint;
    Vector3 m_halfLength;

    Matrix3 m_matEigVectors;
};

class MORTY_API MBoundsSphere : public MIBounds
{
public:
    MBoundsSphere();

    MBoundsSphere(const Vector3& v3CenterPoint, const float& fRadius);

    void
    SetPoints(const void* vPoints, const size_t& unArrayLength, const uint32_t& unOffset, const uint32_t& unDataSize)
    {
        SetPoints((const MByte*) vPoints, unArrayLength, unOffset, unDataSize);
    }

    void
    SetPoints(const MByte* vPoints, const size_t& unArrayLength, const uint32_t& unOffset, const uint32_t& unDataSize);

    void SetPoints(const std::vector<Vector3>& vPoints);

    void AddPoint(const Vector3& pos);

    bool IsContain(const Vector3& pos);

    bool IsIntersect(const MBoundsSphere& other) const;

public:
    virtual flatbuffers::Offset<void> Serialize(flatbuffers::FlatBufferBuilder& fbb) const;

    virtual void                      Deserialize(const void* pBufferPointer);

public:
    Vector3 m_centerPoint;
    float   m_radius;
};

}// namespace morty