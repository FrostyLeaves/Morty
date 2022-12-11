#include "Basic/MCameraFrustum.h"
#include "Math/MMath.h"
#include "Math/Vector.h"
#include "Basic/MViewport.h"

MCameraFrustum::MCameraFrustum()
	: m_vPlanes()
{
}

MCameraFrustum::~MCameraFrustum()
{
}

const MPlane& MCameraFrustum::GetPlane(const size_t& idx) const
{
	return m_vPlanes[(std::min)(idx, size_t(6))];
}

void MCameraFrustum::UpdateFromCameraInvProj(const Matrix4& m4CameraInvProj)
{
	const Matrix4& m4 = m4CameraInvProj;

	Vector4 c0 = m4.Row(0);
	Vector4 c1 = m4.Row(1);
	Vector4 c2 = m4.Row(2);
	Vector4 c3 = m4.Row(3);
	
	m_vPlanes[0].m_v4Plane = -c3 - c0;	//Left
	m_vPlanes[1].m_v4Plane = c0 - c3;	//Right
	m_vPlanes[2].m_v4Plane = c1 - c3;	//Up
	m_vPlanes[3].m_v4Plane = -c3 - c1;	//Down
	m_vPlanes[4].m_v4Plane = -c2;		//Near
	m_vPlanes[5].m_v4Plane = c2 - c3;	//Far

}

MCameraFrustum::MEContainType MCameraFrustum::ContainTest(const Vector3& position)
{
	for (uint32_t i = 0; i < 6; ++i)
	{
		if (m_vPlanes[i].IsOnFront(position))
			return MEContainType::EOUTSIDE;
	}

	return MEContainType::EINSIDE;
}

MCameraFrustum::MEContainType MCameraFrustum::ContainTest(const MBoundsSphere& sphere)
{
	bool bIntersectable = false;

	for (uint32_t i = 0; i < 6; ++i)
	{
		float fDistance = m_vPlanes[i].GetDistance(sphere.m_v3CenterPoint);
		if (fDistance > 0.0f)
		{
			if (fDistance > sphere.m_fRadius)
				return MEContainType::EOUTSIDE;
			
			bIntersectable = true;
		}
	}

	if (bIntersectable)
		return MEContainType::EINTERSECT;

	return MEContainType::EINSIDE;
}

MCameraFrustum::MEContainType MCameraFrustum::ContainTest(const MBoundsAABB& aabb)
{
	bool bIntersectable = false;

	Vector3 v3NVertex, v3PVertex;

	const Vector3& v3Max = aabb.m_v3MaxPoint;
	const Vector3& v3Min = aabb.m_v3MinPoint;
	for (uint32_t i = 0; i < 6; ++i)
	{
		const Vector3& v3Normal = m_vPlanes[i].m_v4Plane.GetVector3();

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

		if (m_vPlanes[i].IsOnFront(v3NVertex))
			return MEContainType::EOUTSIDE;

 		if (m_vPlanes[i].IsOnFront(v3PVertex))
 			bIntersectable = true;
	}

	if (bIntersectable)
		return MEContainType::EINTERSECT;
	
	return MEContainType::EINSIDE;
}

MCameraFrustum::MEContainType MCameraFrustum::ContainTest(const MBoundsAABB& aabb, const Vector3& v3Direction)
{
	Vector3 v3NVertex;

	const Vector3& v3Max = aabb.m_v3MaxPoint;
	const Vector3& v3Min = aabb.m_v3MinPoint;
	for (uint32_t i = 0; i < 6; ++i)
	{
		const Vector3& v3Normal = m_vPlanes[i].m_v4Plane.GetVector3();

		if (v3Direction * v3Normal >= 0.0f)
		{
			for (uint32_t n = 0; n < 3; ++n)
			{
				if (v3Normal.m[n] > 0.0f)
				{
					v3NVertex.m[n] = v3Min.m[n];
				}
				else
				{
					v3NVertex.m[n] = v3Max.m[n];
				}
			}

			if (m_vPlanes[i].IsOnFront(v3NVertex))
				return MEContainType::EOUTSIDE;
		}
	}

	return MEContainType::ENOTOUTSIDE;
}

std::vector<MCameraFrustum> MCameraFrustum::CutFrustum(const std::vector<float>& percent)
{
	size_t nSize = percent.size();

	std::vector<MCameraFrustum> vResult(nSize);

	Vector3 dir = m_vPlanes[4].m_v4Plane;

	float fNearPercent = 0.0f;
	float fFarPercent = 1.0f;
	float fNearFarDistance = fabs(m_vPlanes[5].m_v4Plane.w - m_vPlanes[4].m_v4Plane.w);

	for (size_t nIdx = 0; nIdx < nSize; ++nIdx)
	{
		MCameraFrustum& cf = vResult[nIdx];
		cf = *this;

		fFarPercent = fabs(fFarPercent - percent[nIdx]);
		cf.m_vPlanes[4].MoveInNormal(-fNearFarDistance * fNearPercent);
		cf.m_vPlanes[5].MoveInNormal(-fNearFarDistance * fFarPercent);
		fNearPercent = fabs(fNearPercent + percent[nIdx]);
	}


	return vResult;
}

