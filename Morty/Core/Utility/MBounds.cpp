#include "Utility/MBounds.h"
#include "Eigen/Eigenvalues"
#include "Math/MMath.h"
#include "Math/Matrix.h"
#include "Utility/MTimer.h"

#include <float.h>

#include "Flatbuffer/MBoundsAABB_generated.h"
#include "Flatbuffer/MBoundsOBB_generated.h"
#include "Flatbuffer/MBoundsSphere_generated.h"

using namespace morty;

MBoundsOBB::MBoundsOBB(const Vector3* vPoints, const uint32_t& unArrayLength)
{
    SetPoints((MByte*) vPoints, unArrayLength, 0, sizeof(Vector3));
}

void MBoundsOBB::SetPoints(
        const MByte*    vPoints,
        const uint32_t& unArrayLength,
        const uint32_t& unOffset,
        const uint32_t& unDataSize
)
{
    Vector3      v3Average;

    float        cov_xx = 0;
    float        cov_yy = 0;
    float        cov_zz = 0;
    float        cov_xy = 0;
    float        cov_xz = 0;
    float        cov_yz = 0;

    const MByte* pointer = nullptr;

    pointer = vPoints;
    for (uint32_t i = 0; i < unArrayLength; ++i, pointer += unDataSize)
    {
        const Vector3& pos = *(Vector3*) (pointer + unOffset);
        v3Average += pos;
    }

    v3Average /= unArrayLength;

    pointer = vPoints;
    for (uint32_t i = 0; i < unArrayLength; ++i, pointer += unDataSize)
    {
        const Vector3& pos = *(Vector3*) (pointer + unOffset);
        cov_xx += (pos.x - v3Average.x) * (pos.x - v3Average.x);
        cov_yy += (pos.y - v3Average.y) * (pos.y - v3Average.y);
        cov_zz += (pos.z - v3Average.z) * (pos.z - v3Average.z);
        cov_xy += (pos.x - v3Average.x) * (pos.y - v3Average.y);
        cov_xz += (pos.x - v3Average.x) * (pos.z - v3Average.z);
        cov_yz += (pos.y - v3Average.y) * (pos.z - v3Average.z);
    }

    cov_xx /= (unArrayLength);
    cov_yy /= (unArrayLength);
    cov_zz /= (unArrayLength);
    cov_xy /= (unArrayLength);
    cov_xz /= (unArrayLength);
    cov_yz /= (unArrayLength);


    Eigen::Matrix3d matCov;
    matCov << cov_xx, cov_xy, cov_xz, cov_xy, cov_yy, cov_yz, cov_xz, cov_yz, cov_zz;
    Eigen::EigenSolver<Eigen::Matrix3d> es(matCov);

    Eigen::Matrix3d                     matTemp       = es.pseudoEigenvectors();
    Eigen::Matrix3d                     matEigVectors = matTemp.transpose();

    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < 3; ++j)
        {
            m_matEigVectors.m[i][j] = *(matEigVectors.data() + i * 3 + j);
        }
    }

    Vector3 v3MinPoint, v3MaxPoint;
    pointer = vPoints;
    for (uint32_t i = 0; i < unArrayLength; ++i, pointer += unDataSize)
    {
        const Vector3& pos = *(Vector3*) (pointer + unOffset);
        v3MinPoint = v3MaxPoint = pos * m_matEigVectors;
        break;
    }

    pointer = vPoints;
    for (uint32_t i = 0; i < unArrayLength; ++i, pointer += unDataSize)
    {
        const Vector3& pos = *(Vector3*) (pointer + unOffset);
        Vector3        mp  = pos * m_matEigVectors;

        if (v3MaxPoint.x < mp.x)
        {
            v3MaxPoint.x = mp.x;
            m_maxPoint.x = pos.x;
        }
        else if (v3MinPoint.x > mp.x)
        {
            v3MinPoint.x = mp.x;
            m_minPoint.x = pos.x;
        }

        if (v3MaxPoint.y < mp.y)
        {
            v3MaxPoint.y = mp.y;
            m_maxPoint.y = pos.y;
        }
        else if (v3MinPoint.y > mp.y)
        {
            v3MinPoint.y = mp.y;
            m_minPoint.y = pos.y;
        }

        if (v3MaxPoint.z < mp.z)
        {
            v3MaxPoint.z = mp.z;
            m_maxPoint.z = pos.z;
        }
        else if (v3MinPoint.z > mp.z)
        {
            v3MinPoint.z = mp.z;
            m_minPoint.z = pos.z;
        }
    }
    m_minPoint = v3MinPoint;
    m_maxPoint = v3MaxPoint;

    m_centerPoint = (m_maxPoint + m_minPoint) * 0.5;
    m_halfLength  = (v3MaxPoint - v3MinPoint) * 0.5;
}

flatbuffers::Offset<void> MBoundsOBB::Serialize(flatbuffers::FlatBufferBuilder& fbb) const
{
    fbs::MBoundsOBBBuilder builder(fbb);

    builder.add_matrix(m_matEigVectors.Serialize(fbb));
    builder.add_min(m_minPoint.Serialize(fbb));
    builder.add_max(m_maxPoint.Serialize(fbb));

    return builder.Finish().Union();
}

void MBoundsOBB::Deserialize(const void* pBufferPointer)
{
    const fbs::MBoundsOBB* fbData =
            reinterpret_cast<const fbs::MBoundsOBB*>(pBufferPointer);

    m_matEigVectors.Deserialize(fbData->matrix());
    m_minPoint.Deserialize(fbData->min());
    m_maxPoint.Deserialize(fbData->max());

    m_centerPoint = (m_maxPoint + m_minPoint) * 0.5;
    m_halfLength  = (m_maxPoint - m_minPoint) * 0.5;
}

Vector3 MBoundsOBB::ConvertToOBB(const Vector3& v3Pos) const
{
    return v3Pos * m_matEigVectors;
}

Vector3 MBoundsOBB::ConvertFromOBB(const Vector3& v3Pos) const
{
    return v3Pos * m_matEigVectors.Inverse();
}

MBoundsAABB::MBoundsAABB(const std::vector<Vector3>& vPoints) { SetPoints(vPoints); }

MBoundsAABB::MBoundsAABB(const MBoundsAABB& aabb, const Matrix4& matWorld)
{
    Vector3 a = matWorld * Vector3(aabb.m_centerPoint.x + aabb.m_halfLength.x,
                                   aabb.m_centerPoint.y + aabb.m_halfLength.y,
                                   aabb.m_centerPoint.z + aabb.m_halfLength.z);
    Vector3 b = matWorld * Vector3(aabb.m_centerPoint.x - aabb.m_halfLength.x,
                                   aabb.m_centerPoint.y - aabb.m_halfLength.y,
                                   aabb.m_centerPoint.z - aabb.m_halfLength.z);

    std::vector<Vector3> vPoints = {
            Vector3(a.x, a.y, a.z),
            Vector3(a.x, a.y, b.z),
            Vector3(a.x, b.y, a.z),
            Vector3(a.x, b.y, b.z),
            Vector3(b.x, a.y, a.z),
            Vector3(b.x, a.y, b.z),
            Vector3(b.x, b.y, a.z),
            Vector3(b.x, b.y, b.z),
    };

    SetPoints(vPoints);
}

MBoundsAABB::MBoundsAABB(const std::vector<Vector3>& vPoints, const Matrix4& matWorld)
{
    std::vector<Vector3> vWorldPoints(vPoints.size());
    for (uint32_t i = 0; i < vPoints.size(); ++i) vWorldPoints[i] = matWorld * vPoints[i];

    SetPoints(vPoints);
}

MBoundsAABB::MBoundsAABB(const Vector3& min, const Vector3& max) { SetMinMax(min, max); }

void MBoundsAABB::SetMinMax(const Vector3& v3Min, const Vector3& v3Max)
{
    m_minPoint = v3Min;
    m_maxPoint = v3Max;

    m_centerPoint = (m_minPoint + m_maxPoint) * 0.5f;
    m_halfLength  = (m_maxPoint - m_minPoint) * 0.5f;
}

void MBoundsAABB::SethalfLength(const Vector3& f3HalfLength)
{
    m_halfLength = f3HalfLength;

    m_minPoint = m_centerPoint - m_halfLength;
    m_maxPoint = m_centerPoint + m_halfLength;
}

void MBoundsAABB::SetPoints(const std::vector<Vector3>& vPoints)
{
    m_maxPoint = Vector3(-FLT_MAX, -FLT_MAX, -FLT_MAX);
    m_minPoint = Vector3(FLT_MAX, FLT_MAX, FLT_MAX);
    for (const Vector3& pos: vPoints)
    {
        if (m_minPoint.x > pos.x) m_minPoint.x = pos.x;
        if (m_minPoint.y > pos.y) m_minPoint.y = pos.y;
        if (m_minPoint.z > pos.z) m_minPoint.z = pos.z;

        if (m_maxPoint.x < pos.x) m_maxPoint.x = pos.x;
        if (m_maxPoint.y < pos.y) m_maxPoint.y = pos.y;
        if (m_maxPoint.z < pos.z) m_maxPoint.z = pos.z;
    }

    m_centerPoint = (m_minPoint + m_maxPoint) * 0.5f;
    m_halfLength  = (m_maxPoint - m_minPoint) * 0.5f;
}

void MBoundsAABB::SetBoundsOBB(
        const Vector3&    v3Origin,
        const Matrix4&    matWorld,
        const MBoundsOBB& obb
)
{
    m_maxPoint = v3Origin;
    m_minPoint = v3Origin;

    Vector3 points[8] = {
            matWorld *
                    obb.ConvertFromOBB(
                            Vector3(obb.m_minPoint.x, obb.m_maxPoint.y, obb.m_maxPoint.z)
                    ),
            matWorld *
                    obb.ConvertFromOBB(
                            Vector3(obb.m_minPoint.x, obb.m_maxPoint.y, obb.m_minPoint.z)
                    ),
            matWorld *
                    obb.ConvertFromOBB(
                            Vector3(obb.m_minPoint.x, obb.m_minPoint.y, obb.m_minPoint.z)
                    ),
            matWorld *
                    obb.ConvertFromOBB(
                            Vector3(obb.m_minPoint.x, obb.m_minPoint.y, obb.m_maxPoint.z)
                    ),
            matWorld *
                    obb.ConvertFromOBB(
                            Vector3(obb.m_maxPoint.x, obb.m_maxPoint.y, obb.m_maxPoint.z)
                    ),
            matWorld *
                    obb.ConvertFromOBB(
                            Vector3(obb.m_maxPoint.x, obb.m_maxPoint.y, obb.m_minPoint.z)
                    ),
            matWorld *
                    obb.ConvertFromOBB(
                            Vector3(obb.m_maxPoint.x, obb.m_minPoint.y, obb.m_minPoint.z)
                    ),
            matWorld *
                    obb.ConvertFromOBB(
                            Vector3(obb.m_maxPoint.x, obb.m_minPoint.y, obb.m_maxPoint.z)
                    ),
    };

    for (uint32_t i = 0; i < 8; ++i)
    {
        if (m_maxPoint.x < points[i].x) m_maxPoint.x = points[i].x;
        if (m_maxPoint.y < points[i].y) m_maxPoint.y = points[i].y;
        if (m_maxPoint.z < points[i].z) m_maxPoint.z = points[i].z;

        if (m_minPoint.x > points[i].x) m_minPoint.x = points[i].x;
        if (m_minPoint.y > points[i].y) m_minPoint.y = points[i].y;
        if (m_minPoint.z > points[i].z) m_minPoint.z = points[i].z;
    }

    m_centerPoint = (m_minPoint + m_maxPoint) * 0.5f;
    m_halfLength  = (m_maxPoint - m_minPoint) * 0.5f;
}

void MBoundsAABB::GetPoints(std::vector<Vector3>& vPoints) const
{
    vPoints.resize(8);

    vPoints[0] =
            m_centerPoint + Vector3(+m_halfLength.x, +m_halfLength.y, -m_halfLength.z);
    vPoints[1] =
            m_centerPoint + Vector3(+m_halfLength.x, +m_halfLength.y, +m_halfLength.z);
    vPoints[2] =
            m_centerPoint + Vector3(+m_halfLength.x, -m_halfLength.y, +m_halfLength.z);
    vPoints[3] =
            m_centerPoint + Vector3(+m_halfLength.x, -m_halfLength.y, -m_halfLength.z);

    vPoints[4] =
            m_centerPoint + Vector3(-m_halfLength.x, +m_halfLength.y, -m_halfLength.z);
    vPoints[5] =
            m_centerPoint + Vector3(-m_halfLength.x, +m_halfLength.y, +m_halfLength.z);
    vPoints[6] =
            m_centerPoint + Vector3(-m_halfLength.x, -m_halfLength.y, +m_halfLength.z);
    vPoints[7] =
            m_centerPoint + Vector3(-m_halfLength.x, -m_halfLength.y, -m_halfLength.z);
}

void MBoundsAABB::UnionMinMax(Vector3& v3Min, Vector3& v3Max) const
{

    if (v3Min.x > m_minPoint.x) v3Min.x = m_minPoint.x;
    if (v3Min.y > m_minPoint.y) v3Min.y = m_minPoint.y;
    if (v3Min.z > m_minPoint.z) v3Min.z = m_minPoint.z;

    if (v3Max.x < m_maxPoint.x) v3Max.x = m_maxPoint.x;
    if (v3Max.y < m_maxPoint.y) v3Max.y = m_maxPoint.y;
    if (v3Max.z < m_maxPoint.z) v3Max.z = m_maxPoint.z;
}

MBoundsAABB MBoundsAABB::IntersectAABB(const MBoundsAABB& aabb) const
{
    MBoundsAABB result;

    Vector3     min = m_minPoint;
    Vector3     max = m_maxPoint;

    for (size_t nIdx = 0; nIdx < 3; ++nIdx)
    {
        min.m[nIdx] = (std::max)(min.m[nIdx], aabb.m_minPoint.m[nIdx]);
        max.m[nIdx] = (std::min)(max.m[nIdx], aabb.m_maxPoint.m[nIdx]);

        min.m[nIdx] = (std::min)(min.m[nIdx], max.m[nIdx]);
    }

    result.SetMinMax(min, max);
    return result;
}

bool MBoundsAABB::IsIntersect(const MBoundsAABB& aabb) const
{
    bool bXIntersect =
            (aabb.m_minPoint.x <= m_maxPoint.x) && (m_minPoint.x <= aabb.m_maxPoint.x);
    bool bYIntersect =
            (aabb.m_minPoint.y <= m_maxPoint.y) && (m_minPoint.y <= aabb.m_maxPoint.y);
    bool bZIntersect =
            (aabb.m_minPoint.z <= m_maxPoint.z) && (m_minPoint.z <= aabb.m_maxPoint.z);

    return bXIntersect && bYIntersect && bZIntersect;
}

MBoundsSphere MBoundsAABB::ToSphere() const
{
    return MBoundsSphere(m_centerPoint, m_halfLength.Length());
}

flatbuffers::Offset<void> MBoundsAABB::Serialize(flatbuffers::FlatBufferBuilder& fbb
) const
{
    fbs::MBoundsAABBBuilder builder(fbb);

    builder.add_min(m_minPoint.Serialize(fbb));
    builder.add_max(m_maxPoint.Serialize(fbb));

    return builder.Finish().Union();
}

void MBoundsAABB::Deserialize(const void* pBufferPointer)
{
    const fbs::MBoundsAABB* fbData =
            reinterpret_cast<const fbs::MBoundsAABB*>(pBufferPointer);

    m_minPoint.Deserialize(fbData->min());
    m_maxPoint.Deserialize(fbData->max());

    SetMinMax(m_minPoint, m_maxPoint);
}

class MPointsSphere
{
public:
    MPointsSphere() {}

    void          SetPoints(const std::vector<Vector3>& vPoints) { m_points = vPoints; }

    void          RandomSwap();

    MBoundsSphere GetMinSurroundBall();

    MBoundsSphere GetMinSurroundBall(const uint32_t& idx1);

    MBoundsSphere GetMinSurroundBall(const uint32_t& idx1, const uint32_t& idx2);

    MBoundsSphere
    GetMinSurroundBall(const uint32_t& idx1, const uint32_t& idx2, const uint32_t& idx3);

    Vector3 GetBallCenter(
            const Vector3& p1,
            const Vector3& p2,
            const Vector3& p3,
            const Vector3& p4
    );

public:
    std::vector<Vector3> m_points;
};

MBoundsSphere::MBoundsSphere(const Vector3& v3CenterPoint, const float& fRadius)
    : m_centerPoint(v3CenterPoint)
    , m_radius(fRadius)
{}

MBoundsSphere::MBoundsSphere()
    : m_centerPoint()
    , m_radius(0.0f)
{}

void MBoundsSphere::SetPoints(const std::vector<Vector3>& vPoints)
{
    SetPoints(vPoints.data(), vPoints.size(), 0, sizeof(Vector3));
}

void MBoundsSphere::SetPoints(
        const MByte*    vPoints,
        const size_t&   unArrayLength,
        const uint32_t& unOffset,
        const uint32_t& unDataSize
)
{

    Vector3      v3Min(FLT_MAX, FLT_MAX, FLT_MAX);
    Vector3      v3Max(-FLT_MAX, -FLT_MAX, -FLT_MAX);

    const MByte* vPointer = vPoints;
    for (uint32_t i = 0; i < unArrayLength; ++i, vPointer += unDataSize)
    {
        const Vector3& pos = *reinterpret_cast<const Vector3*>(vPointer + unOffset);
        if (v3Min.x > pos.x) v3Min.x = pos.x;
        if (v3Min.y > pos.y) v3Min.y = pos.y;
        if (v3Min.z > pos.z) v3Min.z = pos.z;

        if (v3Max.x < pos.x) v3Max.x = pos.x;
        if (v3Max.y < pos.y) v3Max.y = pos.y;
        if (v3Max.z < pos.z) v3Max.z = pos.z;
    }

    m_centerPoint = (v3Max + v3Min) * 0.5f;

    Vector3 v3Length = v3Max - v3Min;
    if (v3Length.x >= v3Length.y && v3Length.x >= v3Length.z)
        m_radius = v3Length.x * 0.5f;
    else if (v3Length.y >= v3Length.z)
        m_radius = v3Length.y * 0.5f;
    else
        m_radius = v3Length.z * 0.5f;

    vPointer = vPoints;
    for (uint32_t i = 0; i < unArrayLength; ++i, vPointer += unDataSize)
    {
        const Vector3& pos = *(Vector3*) (vPointer + unOffset);

        AddPoint(pos);
    }
}

void MBoundsSphere::AddPoint(const Vector3& pos)
{
    float fLength = (pos - m_centerPoint).Length();
    if (fLength > m_radius)
    {
        Vector3 direct = pos - m_centerPoint;
        direct.Normalize();
        m_centerPoint = (m_centerPoint + direct * (fLength - m_radius) * 0.5f);
        m_radius      = (fLength + m_radius) * 0.5f;
    }
}

bool MBoundsSphere::IsContain(const Vector3& pos)
{
    return (pos - m_centerPoint).Length() <= m_radius;
}

bool MBoundsSphere::IsIntersect(const MBoundsSphere& other) const
{
    return (m_centerPoint - other.m_centerPoint).Length() < (m_radius + other.m_radius);
}

flatbuffers::Offset<void> MBoundsSphere::Serialize(flatbuffers::FlatBufferBuilder& fbb
) const
{
    fbs::MBoundsSphereBuilder builder(fbb);

    builder.add_center(m_centerPoint.Serialize(fbb));
    builder.add_radius(m_radius);

    return builder.Finish().Union();
}

void MBoundsSphere::Deserialize(const void* pBufferPointer)
{
    const fbs::MBoundsSphere* fbData =
            reinterpret_cast<const fbs::MBoundsSphere*>(pBufferPointer);

    m_centerPoint.Deserialize(fbData->center());
    m_radius = fbData->radius();
}

void MPointsSphere::RandomSwap()
{
    size_t  nSize = m_points.size();

    size_t  unIndex = 0;
    Vector3 v3Temp;
    for (size_t i = 0; i < nSize; ++i)
    {
        unIndex = MMath::RandInt(0, static_cast<int>(nSize) - 1);

        v3Temp            = m_points[i];
        m_points[i]       = m_points[unIndex];
        m_points[unIndex] = v3Temp;
    }
}

MBoundsSphere MPointsSphere::GetMinSurroundBall()
{
    RandomSwap();

    MBoundsSphere sphere(m_points[0], 0);
    for (uint32_t i = 1; i < m_points.size(); ++i)
    {
        if (!sphere.IsContain(m_points[i])) { sphere = GetMinSurroundBall(i); }
    }

    return sphere;
}

MBoundsSphere MPointsSphere::GetMinSurroundBall(const uint32_t& idx1)
{
    MBoundsSphere sphere(m_points[idx1], 0);

    for (uint32_t i = 0; i < idx1; ++i)
    {
        if (!sphere.IsContain(m_points[i])) { sphere = GetMinSurroundBall(idx1, i); }
    }
    return sphere;
}

MBoundsSphere
MPointsSphere::GetMinSurroundBall(const uint32_t& idx1, const uint32_t& idx2)
{
    MBoundsSphere sphere(
            (m_points[idx1] + m_points[idx2]) * 0.5f,
            (m_points[idx1] - m_points[idx2]).Length()
    );

    for (uint32_t i = 1; i < idx2; ++i)
    {
        if (!sphere.IsContain(m_points[i]))
        {
            sphere = GetMinSurroundBall(idx1, idx2, i);// i idx1 idx2
        }
    }
    return sphere;
}

MBoundsSphere MPointsSphere::GetMinSurroundBall(
        const uint32_t& idx1,
        const uint32_t& idx2,
        const uint32_t& idx3
)
{
    MBoundsSphere sphere;

    Vector3       v3Center = (m_points[idx1] + m_points[idx2] + m_points[idx3]) / 3.0f;
    float         fRadius  = (m_points[idx2] - sphere.m_centerPoint).Length();


    for (uint32_t i = 0; i < idx3; ++i)
    {
        if (!sphere.IsContain(m_points[i]))
        {
            Vector3 v3NewCenter = GetBallCenter(
                    m_points[i],
                    m_points[idx1],
                    m_points[idx2],
                    m_points[idx3]
            );
            float fNewRadius = (v3NewCenter - m_points[idx2]).Length();
            if (fNewRadius > fRadius)
            {
                v3Center = v3NewCenter;
                fRadius  = fNewRadius;
                sphere   = MBoundsSphere(v3Center, fRadius);
            }
        }
    }
    return sphere;
}

Vector3 MPointsSphere::GetBallCenter(
        const Vector3& p1,
        const Vector3& p2,
        const Vector3& p3,
        const Vector3& p4
)
{
    float a11, a12, a13, a21, a22, a23, a31, a32, a33, b1, b2, b3, d, d1, d2, d3;
    a11 = 2 * (p2.x - p1.x);
    a12 = 2 * (p2.y - p1.y);
    a13 = 2 * (p2.z - p1.z);
    a21 = 2 * (p3.x - p2.x);
    a22 = 2 * (p3.y - p2.y);
    a23 = 2 * (p3.z - p2.z);
    a31 = 2 * (p4.x - p3.x);
    a32 = 2 * (p4.y - p3.y);
    a33 = 2 * (p4.z - p3.z);
    b1  = p2.x * p2.x - p1.x * p1.x + p2.y * p2.y - p1.y * p1.y + p2.z * p2.z -
         p1.z * p1.z;
    b2 = p3.x * p3.x - p2.x * p2.x + p3.y * p3.y - p2.y * p2.y + p3.z * p3.z -
         p2.z * p2.z;
    b3 = p4.x * p4.x - p3.x * p3.x + p4.y * p4.y - p3.y * p3.y + p4.z * p4.z -
         p3.z * p3.z;
    d = a11 * a22 * a33 + a12 * a23 * a31 + a13 * a21 * a32 - a11 * a23 * a32 -
        a12 * a21 * a33 - a13 * a22 * a31;
    d1 = b1 * a22 * a33 + a12 * a23 * b3 + a13 * b2 * a32 - b1 * a23 * a32 -
         a12 * b2 * a33 - a13 * a22 * b3;
    d2 = a11 * b2 * a33 + b1 * a23 * a31 + a13 * a21 * b3 - a11 * a23 * b3 -
         b1 * a21 * a33 - a13 * b2 * a31;
    d3 = a11 * a22 * b3 + a12 * b2 * a31 + b1 * a21 * a32 - a11 * b2 * a32 -
         a12 * a21 * b3 - b1 * a22 * a31;

    return Vector3(d1 / d, d2 / d, d3 / d);
}
