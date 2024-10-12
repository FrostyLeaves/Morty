#include "Basic/MCameraFrustum.h"
#include "Basic/MViewport.h"
#include "Math/MMath.h"
#include "Math/Vector.h"

using namespace morty;

MCameraFrustum::MCameraFrustum()
    : m_planes()
{}

MCameraFrustum::~MCameraFrustum() {}

MPlane MCameraFrustum::GetPlane(const size_t& idx) const { return m_planes[(std::min)(idx, static_cast<size_t>(6))]; }

void   MCameraFrustum::UpdateFromCameraInvProj(const Matrix4& m4CameraInvProj)
{
    const Matrix4& m4 = m4CameraInvProj;

    const Vector4  c0 = m4.Row(0);
    const Vector4  c1 = m4.Row(1);
    const Vector4  c2 = m4.Row(2);
    const Vector4  c3 = m4.Row(3);

    m_planes[0].m_plane = -c3 - c0;//Left
    m_planes[1].m_plane = c0 - c3; //Right
    m_planes[2].m_plane = c1 - c3; //Up
    m_planes[3].m_plane = -c3 - c1;//Down
    m_planes[4].m_plane = -c2;     //Near
    m_planes[5].m_plane = c2 - c3; //Far
}

MCameraFrustum::MEContainType MCameraFrustum::ContainTest(const Vector3& position) const
{
    for (uint32_t i = 0; i < 6; ++i)
    {
        if (m_planes[i].IsOnFront(position)) return MEContainType::EOUTSIDE;
    }

    return MEContainType::EINSIDE;
}

MCameraFrustum::MEContainType MCameraFrustum::ContainTest(const MBoundsSphere& sphere) const
{
    bool bIntersectable = false;

    for (uint32_t i = 0; i < 6; ++i)
    {
        float fDistance = m_planes[i].GetDistance(sphere.m_centerPoint);
        if (fDistance > 0.0f)
        {
            if (fDistance > sphere.m_radius) return MEContainType::EOUTSIDE;

            bIntersectable = true;
        }
    }

    if (bIntersectable) return MEContainType::EINTERSECT;

    return MEContainType::EINSIDE;
}

MCameraFrustum::MEContainType MCameraFrustum::ContainTest(const MBoundsAABB& aabb) const
{
    bool           bIntersectable = false;

    Vector3        v3NVertex, v3PVertex;

    const Vector3& v3Max = aabb.m_maxPoint;
    const Vector3& v3Min = aabb.m_minPoint;
    for (uint32_t i = 0; i < 6; ++i)
    {
        const Vector3& v3Normal = m_planes[i].m_plane.GetVector3();

        for (uint32_t n = 0; n < 3; ++n)
        {
            if (v3Normal.m[n] > 0.0f)
            {
                v3NVertex.m[n] = v3Min.m[n];
                v3PVertex.m[n] = v3Max.m[n];
            }
            else
            {
                v3NVertex.m[n] = v3Max.m[n];
                v3PVertex.m[n] = v3Min.m[n];
            }
        }

        if (m_planes[i].IsOnFront(v3NVertex)) return MEContainType::EOUTSIDE;

        if (m_planes[i].IsOnFront(v3PVertex)) bIntersectable = true;
    }

    if (bIntersectable) return MEContainType::EINTERSECT;

    return MEContainType::EINSIDE;
}

MCameraFrustum::MEContainType MCameraFrustum::ContainTest(const MBoundsAABB& aabb, const Vector3& v3Direction) const
{
    Vector3        v3NVertex;

    const Vector3& v3Max = aabb.m_maxPoint;
    const Vector3& v3Min = aabb.m_minPoint;
    for (uint32_t i = 0; i < 6; ++i)
    {
        const Vector3& v3Normal = m_planes[i].m_plane.GetVector3();

        if (v3Direction * v3Normal >= 0.0f)
        {
            for (uint32_t n = 0; n < 3; ++n)
            {
                if (v3Normal.m[n] > 0.0f) { v3NVertex.m[n] = v3Min.m[n]; }
                else { v3NVertex.m[n] = v3Max.m[n]; }
            }

            if (m_planes[i].IsOnFront(v3NVertex)) return MEContainType::EOUTSIDE;
        }
    }

    return MEContainType::ENOTOUTSIDE;
}

std::vector<MCameraFrustum> MCameraFrustum::CutFrustum(const std::vector<float>& percent)
{
    size_t                      nSize = percent.size();

    std::vector<MCameraFrustum> vResult(nSize);

    float                       fNearPercent     = 0.0f;
    float                       fFarPercent      = 1.0f;
    float                       fNearFarDistance = fabs(m_planes[5].m_plane.w - m_planes[4].m_plane.w);

    for (size_t nIdx = 0; nIdx < nSize; ++nIdx)
    {
        MCameraFrustum& cf = vResult[nIdx];
        cf                 = *this;

        fFarPercent = fabs(fFarPercent - percent[nIdx]);
        cf.m_planes[4].MoveInNormal(-fNearFarDistance * fNearPercent);
        cf.m_planes[5].MoveInNormal(-fNearFarDistance * fFarPercent);
        fNearPercent = fabs(fNearPercent + percent[nIdx]);
    }


    return vResult;
}
