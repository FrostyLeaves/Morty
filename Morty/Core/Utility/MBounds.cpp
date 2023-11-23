#include "Utility/MBounds.h"
#include "Eigen/Eigenvalues"
#include "Math/Matrix.h"
#include "Math/MMath.h"
#include "Utility/MTimer.h"

#include <float.h>

#include "Flatbuffer/MBoundsAABB_generated.h"
#include "Flatbuffer/MBoundsOBB_generated.h"
#include "Flatbuffer/MBoundsSphere_generated.h"

MBoundsOBB::MBoundsOBB(const Vector3* vPoints, const uint32_t& unArrayLength)
{
	SetPoints((MByte*)vPoints, unArrayLength, 0, sizeof(Vector3));
}

void MBoundsOBB::SetPoints(const MByte* vPoints, const uint32_t& unArrayLength, const uint32_t& unOffset, const uint32_t& unDataSize)
{
	Vector3 v3Average;

	float cov_xx = 0;
	float cov_yy = 0;
	float cov_zz = 0;
	float cov_xy = 0;
	float cov_xz = 0;
	float cov_yz = 0;

	const MByte* pointer = nullptr;

	pointer = vPoints;
	for(uint32_t i = 0; i < unArrayLength; ++i, pointer += unDataSize)
	{
		const Vector3& pos = *(Vector3*)(pointer + unOffset);
		v3Average += pos;
	}

	v3Average /= unArrayLength;

	pointer = vPoints;
	for (uint32_t i = 0; i < unArrayLength; ++i, pointer += unDataSize)
	{
		const Vector3& pos = *(Vector3*)(pointer + unOffset);
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

	Eigen::Matrix3d matTemp = es.pseudoEigenvectors();
	Eigen::Matrix3d matEigVectors = matTemp.transpose();

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
		const Vector3& pos = *(Vector3*)(pointer + unOffset);
		v3MinPoint = v3MaxPoint = pos * m_matEigVectors;
		break;
	}

	pointer = vPoints;
	for (uint32_t i = 0; i < unArrayLength; ++i, pointer += unDataSize)
	{
		const Vector3& pos = *(Vector3*)(pointer + unOffset);
		Vector3 mp = pos * m_matEigVectors;

		if (v3MaxPoint.x < mp.x)
		{
			v3MaxPoint.x = mp.x;
			m_v3MaxPoint.x = pos.x;
		}
		else if (v3MinPoint.x > mp.x)
		{
			v3MinPoint.x = mp.x;
			m_v3MinPoint.x = pos.x;
		}

		if (v3MaxPoint.y < mp.y)
		{
			v3MaxPoint.y = mp.y;
			m_v3MaxPoint.y = pos.y;
		}
		else if (v3MinPoint.y > mp.y)
		{
			v3MinPoint.y = mp.y;
			m_v3MinPoint.y = pos.y;
		}

		if (v3MaxPoint.z < mp.z)
		{
			v3MaxPoint.z = mp.z;
			m_v3MaxPoint.z = pos.z;
		}
		else if (v3MinPoint.z > mp.z)
		{
			v3MinPoint.z = mp.z;
			m_v3MinPoint.z = pos.z;
		}
	}
	m_v3MinPoint = v3MinPoint;
	m_v3MaxPoint = v3MaxPoint;

	m_v3CenterPoint = (m_v3MaxPoint + m_v3MinPoint) * 0.5;
	m_v3HalfLength = (v3MaxPoint - v3MinPoint) * 0.5;
}

flatbuffers::Offset<void> MBoundsOBB::Serialize(flatbuffers::FlatBufferBuilder& fbb) const
{
	mfbs::MBoundsOBBBuilder builder(fbb);

	builder.add_matrix(m_matEigVectors.Serialize(fbb));
	builder.add_min(m_v3MinPoint.Serialize(fbb));
	builder.add_max(m_v3MaxPoint.Serialize(fbb));

	return builder.Finish().Union();
}

void MBoundsOBB::Deserialize(const void* pBufferPointer)
{
	const mfbs::MBoundsOBB* fbData = reinterpret_cast<const mfbs::MBoundsOBB*>(pBufferPointer);

	m_matEigVectors.Deserialize(fbData->matrix());
	m_v3MinPoint.Deserialize(fbData->min());
	m_v3MaxPoint.Deserialize(fbData->max());

	m_v3CenterPoint = (m_v3MaxPoint + m_v3MinPoint) * 0.5;
	m_v3HalfLength = (m_v3MaxPoint - m_v3MinPoint) * 0.5;
}

Vector3 MBoundsOBB::ConvertToOBB(const Vector3& v3Pos) const
{
	return v3Pos * m_matEigVectors;
}

Vector3 MBoundsOBB::ConvertFromOBB(const Vector3& v3Pos) const
{
	return v3Pos * m_matEigVectors.Inverse();
}

MBoundsAABB::MBoundsAABB(const std::vector<Vector3>& vPoints)
{
	SetPoints(vPoints);
}

MBoundsAABB::MBoundsAABB(const MBoundsAABB& aabb, const Matrix4& matWorld)
{
	Vector3 a = matWorld * Vector3(aabb.m_v3CenterPoint.x + aabb.m_v3HalfLength.x, aabb.m_v3CenterPoint.y + aabb.m_v3HalfLength.y, aabb.m_v3CenterPoint.z + aabb.m_v3HalfLength.z);
	Vector3 b = matWorld * Vector3(aabb.m_v3CenterPoint.x - aabb.m_v3HalfLength.x, aabb.m_v3CenterPoint.y - aabb.m_v3HalfLength.y, aabb.m_v3CenterPoint.z - aabb.m_v3HalfLength.z);

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
	for (uint32_t i = 0; i < vPoints.size(); ++i)
		vWorldPoints[i] = matWorld * vPoints[i];

	SetPoints(vPoints);
}

MBoundsAABB::MBoundsAABB(const Vector3& min, const Vector3& max)
{
	SetMinMax(min, max);
}

void MBoundsAABB::SetMinMax(const Vector3& v3Min, const Vector3& v3Max)
{
	m_v3MinPoint = v3Min;
	m_v3MaxPoint = v3Max;

	m_v3CenterPoint = (m_v3MinPoint + m_v3MaxPoint) * 0.5f;
	m_v3HalfLength = (m_v3MaxPoint - m_v3MinPoint) * 0.5f;
}

void MBoundsAABB::SetPoints(const std::vector<Vector3>& vPoints)
{
	m_v3MaxPoint = Vector3(-FLT_MAX, -FLT_MAX, -FLT_MAX);
	m_v3MinPoint = Vector3(FLT_MAX, FLT_MAX, FLT_MAX);
	for (const Vector3& pos : vPoints)
	{
		if (m_v3MinPoint.x > pos.x)
			m_v3MinPoint.x = pos.x;
		if (m_v3MinPoint.y > pos.y)
			m_v3MinPoint.y = pos.y;
		if (m_v3MinPoint.z > pos.z)
			m_v3MinPoint.z = pos.z;

		if (m_v3MaxPoint.x < pos.x)
			m_v3MaxPoint.x = pos.x;
		if (m_v3MaxPoint.y < pos.y)
			m_v3MaxPoint.y = pos.y;
		if (m_v3MaxPoint.z < pos.z)
			m_v3MaxPoint.z = pos.z;
	}

	m_v3CenterPoint = (m_v3MinPoint + m_v3MaxPoint) * 0.5f;
	m_v3HalfLength = (m_v3MaxPoint - m_v3MinPoint) * 0.5f;
}

void MBoundsAABB::SetBoundsOBB(const Vector3& v3Origin, const Matrix4& matWorld, const MBoundsOBB& obb)
{
	m_v3MaxPoint = v3Origin;
	m_v3MinPoint = v3Origin;

	Vector3 points[8] = {
		matWorld * obb.ConvertFromOBB(Vector3(obb.m_v3MinPoint.x, obb.m_v3MaxPoint.y, obb.m_v3MaxPoint.z)),
		matWorld * obb.ConvertFromOBB(Vector3(obb.m_v3MinPoint.x, obb.m_v3MaxPoint.y, obb.m_v3MinPoint.z)),
		matWorld * obb.ConvertFromOBB(Vector3(obb.m_v3MinPoint.x, obb.m_v3MinPoint.y, obb.m_v3MinPoint.z)),
		matWorld * obb.ConvertFromOBB(Vector3(obb.m_v3MinPoint.x, obb.m_v3MinPoint.y, obb.m_v3MaxPoint.z)),
		matWorld * obb.ConvertFromOBB(Vector3(obb.m_v3MaxPoint.x, obb.m_v3MaxPoint.y, obb.m_v3MaxPoint.z)),
		matWorld * obb.ConvertFromOBB(Vector3(obb.m_v3MaxPoint.x, obb.m_v3MaxPoint.y, obb.m_v3MinPoint.z)),
		matWorld * obb.ConvertFromOBB(Vector3(obb.m_v3MaxPoint.x, obb.m_v3MinPoint.y, obb.m_v3MinPoint.z)),
		matWorld * obb.ConvertFromOBB(Vector3(obb.m_v3MaxPoint.x, obb.m_v3MinPoint.y, obb.m_v3MaxPoint.z)),
	};

	for (uint32_t i = 0; i < 8; ++i)
	{
		if (m_v3MaxPoint.x < points[i].x)
			m_v3MaxPoint.x = points[i].x;
		if (m_v3MaxPoint.y < points[i].y)
			m_v3MaxPoint.y = points[i].y;
		if (m_v3MaxPoint.z < points[i].z)
			m_v3MaxPoint.z = points[i].z;

		if (m_v3MinPoint.x > points[i].x)
			m_v3MinPoint.x = points[i].x;
		if (m_v3MinPoint.y > points[i].y)
			m_v3MinPoint.y = points[i].y;
		if (m_v3MinPoint.z > points[i].z)
			m_v3MinPoint.z = points[i].z;
	}

	m_v3CenterPoint = (m_v3MinPoint + m_v3MaxPoint) * 0.5f;
	m_v3HalfLength = (m_v3MaxPoint - m_v3MinPoint) * 0.5f;
}

void MBoundsAABB::GetPoints(std::vector<Vector3>& vPoints) const
{
	vPoints.resize(8);

	vPoints[0] = m_v3CenterPoint + Vector3(+m_v3HalfLength.x, +m_v3HalfLength.y, -m_v3HalfLength.z);
	vPoints[1] = m_v3CenterPoint + Vector3(+m_v3HalfLength.x, +m_v3HalfLength.y, +m_v3HalfLength.z);
	vPoints[2] = m_v3CenterPoint + Vector3(+m_v3HalfLength.x, -m_v3HalfLength.y, +m_v3HalfLength.z);
	vPoints[3] = m_v3CenterPoint + Vector3(+m_v3HalfLength.x, -m_v3HalfLength.y, -m_v3HalfLength.z);

	vPoints[4] = m_v3CenterPoint + Vector3(-m_v3HalfLength.x, +m_v3HalfLength.y, -m_v3HalfLength.z);
	vPoints[5] = m_v3CenterPoint + Vector3(-m_v3HalfLength.x, +m_v3HalfLength.y, +m_v3HalfLength.z);
	vPoints[6] = m_v3CenterPoint + Vector3(-m_v3HalfLength.x, -m_v3HalfLength.y, +m_v3HalfLength.z);
	vPoints[7] = m_v3CenterPoint + Vector3(-m_v3HalfLength.x, -m_v3HalfLength.y, -m_v3HalfLength.z);
}

void MBoundsAABB::UnionMinMax(Vector3& v3Min, Vector3& v3Max) const
{

	if (v3Min.x > m_v3MinPoint.x)
		v3Min.x = m_v3MinPoint.x;
	if (v3Min.y > m_v3MinPoint.y)
		v3Min.y = m_v3MinPoint.y;
	if (v3Min.z > m_v3MinPoint.z)
		v3Min.z = m_v3MinPoint.z;

	if (v3Max.x < m_v3MaxPoint.x)
		v3Max.x = m_v3MaxPoint.x;
	if (v3Max.y < m_v3MaxPoint.y)
		v3Max.y = m_v3MaxPoint.y;
	if (v3Max.z < m_v3MaxPoint.z)
		v3Max.z = m_v3MaxPoint.z;
}

MBoundsAABB MBoundsAABB::IntersectAABB(const MBoundsAABB& aabb) const
{
	MBoundsAABB result;

	Vector3 min = m_v3MinPoint;
	Vector3 max = m_v3MaxPoint;

	for (size_t nIdx = 0; nIdx < 3; ++nIdx)
	{
		min.m[nIdx] = (std::max)(min.m[nIdx], aabb.m_v3MinPoint.m[nIdx]);
		max.m[nIdx] = (std::min)(max.m[nIdx], aabb.m_v3MaxPoint.m[nIdx]);

		min.m[nIdx] = (std::min)(min.m[nIdx], max.m[nIdx]);
	}

	result.SetMinMax(min, max);
	return result;
}

bool MBoundsAABB::IsIntersect(const MBoundsAABB& aabb) const
{
	bool bXIntersect = (aabb.m_v3MinPoint.x <= m_v3MaxPoint.x) && (m_v3MinPoint.x <= aabb.m_v3MaxPoint.x);
	bool bYIntersect = (aabb.m_v3MinPoint.y <= m_v3MaxPoint.y) && (m_v3MinPoint.y <= aabb.m_v3MaxPoint.y);
	bool bZIntersect = (aabb.m_v3MinPoint.z <= m_v3MaxPoint.z) && (m_v3MinPoint.z <= aabb.m_v3MaxPoint.z);

	return bXIntersect && bYIntersect && bZIntersect;
}

flatbuffers::Offset<void> MBoundsAABB::Serialize(flatbuffers::FlatBufferBuilder& fbb) const
{
	mfbs::MBoundsAABBBuilder builder(fbb);

	builder.add_min(m_v3MinPoint.Serialize(fbb));
	builder.add_max(m_v3MaxPoint.Serialize(fbb));

	return builder.Finish().Union();
}

void MBoundsAABB::Deserialize(const void* pBufferPointer)
{
	const mfbs::MBoundsAABB* fbData = reinterpret_cast<const mfbs::MBoundsAABB*>(pBufferPointer);

	m_v3MinPoint.Deserialize(fbData->min());
	m_v3MaxPoint.Deserialize(fbData->max());

	SetMinMax(m_v3MinPoint, m_v3MaxPoint);
}

class MPointsSphere
{
public:
	MPointsSphere() {}

	void SetPoints(const std::vector<Vector3>& vPoints) { m_vPoints = vPoints; }

	void RandomSwap();

	MBoundsSphere GetMinSurroundBall();
	MBoundsSphere GetMinSurroundBall(const uint32_t& idx1);
	MBoundsSphere GetMinSurroundBall(const uint32_t& idx1, const uint32_t& idx2);
	MBoundsSphere GetMinSurroundBall(const uint32_t& idx1, const uint32_t& idx2, const uint32_t& idx3);

	Vector3 GetBallCenter(const Vector3& p1, const Vector3& p2, const Vector3& p3, const Vector3& p4);

public:

	std::vector<Vector3> m_vPoints;
};

MBoundsSphere::MBoundsSphere(const Vector3& v3CenterPoint, const float& fRadius)
	: m_v3CenterPoint(v3CenterPoint)
	, m_fRadius(fRadius)
{

}

MBoundsSphere::MBoundsSphere()
	: m_v3CenterPoint()
	, m_fRadius(0.0f)
{

}

void MBoundsSphere::SetPoints(const std::vector<Vector3>& vPoints)
{
	SetPoints(vPoints.data(), vPoints.size(), 0, sizeof(Vector3));
}

void MBoundsSphere::SetPoints(const MByte* vPoints, const size_t& unArrayLength, const uint32_t& unOffset, const uint32_t& unDataSize)
{
	
	Vector3 v3Min(FLT_MAX, FLT_MAX, FLT_MAX);
	Vector3 v3Max(-FLT_MAX, -FLT_MAX, -FLT_MAX);

	const MByte* vPointer = vPoints;
	for (uint32_t i = 0; i < unArrayLength; ++i, vPointer += unDataSize)
	{
		const Vector3& pos = *reinterpret_cast<const Vector3*>(vPointer + unOffset);
		if (v3Min.x > pos.x)
			v3Min.x = pos.x;
		if (v3Min.y > pos.y)
			v3Min.y = pos.y;
		if (v3Min.z > pos.z)
			v3Min.z = pos.z;

		if (v3Max.x < pos.x)
			v3Max.x = pos.x;
		if (v3Max.y < pos.y)
			v3Max.y = pos.y;
		if (v3Max.z < pos.z)
			v3Max.z = pos.z;
	}

	m_v3CenterPoint = (v3Max + v3Min) * 0.5f;

	Vector3 v3Length = v3Max - v3Min;
	if (v3Length.x >= v3Length.y && v3Length.x >= v3Length.z)
		m_fRadius = v3Length.x * 0.5f;
	else if (v3Length.y >= v3Length.z)
		m_fRadius = v3Length.y * 0.5f;
	else
		m_fRadius = v3Length.z * 0.5f;

	vPointer = vPoints;
	for (uint32_t i = 0; i < unArrayLength; ++i, vPointer += unDataSize)
	{
		const Vector3& pos = *(Vector3*)(vPointer + unOffset);

		AddPoint(pos);
	}

}

void MBoundsSphere::AddPoint(const Vector3& pos)
{
	float fLength = (pos - m_v3CenterPoint).Length();
	if (fLength > m_fRadius)
	{
		Vector3 direct = pos - m_v3CenterPoint;
		direct.Normalize();
		m_v3CenterPoint = (m_v3CenterPoint + direct * (fLength - m_fRadius) * 0.5f);
		m_fRadius = (fLength + m_fRadius) * 0.5f;
	}
}

bool MBoundsSphere::IsContain(const Vector3& pos)
{
	return (pos - m_v3CenterPoint).Length() <= m_fRadius;
}

flatbuffers::Offset<void> MBoundsSphere::Serialize(flatbuffers::FlatBufferBuilder& fbb) const
{
	mfbs::MBoundsSphereBuilder builder(fbb);

	builder.add_center(m_v3CenterPoint.Serialize(fbb));
	builder.add_radius(m_fRadius);

	return builder.Finish().Union();
}

void MBoundsSphere::Deserialize(const void* pBufferPointer)
{
	const mfbs::MBoundsSphere* fbData = reinterpret_cast<const mfbs::MBoundsSphere*>(pBufferPointer);

	m_v3CenterPoint.Deserialize(fbData->center());
	m_fRadius = fbData->radius();
}

void MPointsSphere::RandomSwap()
{
	size_t nSize = m_vPoints.size();
	
	size_t unIndex = 0;
	Vector3 v3Temp;
	for (size_t i = 0; i < nSize; ++i)
	{
		unIndex = MMath::RandInt(0, static_cast<int>(nSize) - 1);

		v3Temp = m_vPoints[i];
		m_vPoints[i] = m_vPoints[unIndex];
		m_vPoints[unIndex] = v3Temp;
	}
}

MBoundsSphere MPointsSphere::GetMinSurroundBall()
{
	RandomSwap();

	MBoundsSphere sphere(m_vPoints[0], 0);
	for (uint32_t i = 1; i < m_vPoints.size(); ++i)
	{
		if (!sphere.IsContain(m_vPoints[i]))
		{
			sphere = GetMinSurroundBall(i);
		}
	}

	return sphere;
}

MBoundsSphere MPointsSphere::GetMinSurroundBall(const uint32_t& idx1)
{
	MBoundsSphere sphere(m_vPoints[idx1], 0);

	for (uint32_t i = 0; i < idx1; ++i)
	{
		if (!sphere.IsContain(m_vPoints[i])) {
			sphere = GetMinSurroundBall(idx1, i);
		}
	
	}
	return sphere;
}

MBoundsSphere MPointsSphere::GetMinSurroundBall(const uint32_t& idx1, const uint32_t& idx2)
{
	MBoundsSphere sphere((m_vPoints[idx1] + m_vPoints[idx2]) * 0.5f, (m_vPoints[idx1] - m_vPoints[idx2]).Length());

	for (uint32_t i = 1; i < idx2; ++i)
	{
		if (!sphere.IsContain(m_vPoints[i])) {
			sphere = GetMinSurroundBall(idx1, idx2, i);// i idx1 idx2
		}

	}
	return sphere;
}
		
MBoundsSphere MPointsSphere::GetMinSurroundBall(const uint32_t& idx1, const uint32_t& idx2, const uint32_t& idx3)
{
	MBoundsSphere sphere;

	Vector3 v3Center = (m_vPoints[idx1] + m_vPoints[idx2] + m_vPoints[idx3]) / 3.0f;
	float fRadius = (m_vPoints[idx2] - sphere.m_v3CenterPoint).Length();
	

	for (uint32_t i = 0; i < idx3; ++i)
	{
		if (!sphere.IsContain(m_vPoints[i]))
		{
			Vector3 v3NewCenter = GetBallCenter(m_vPoints[i], m_vPoints[idx1], m_vPoints[idx2], m_vPoints[idx3]);
			float fNewRadius = (v3NewCenter - m_vPoints[idx2]).Length();
			if (fNewRadius > fRadius)
			{
				v3Center = v3NewCenter;
				fRadius = fNewRadius;
				sphere = MBoundsSphere(v3Center, fRadius);
			}
		}
	}
	return sphere;
}

Vector3 MPointsSphere::GetBallCenter(const Vector3& p1, const Vector3& p2, const Vector3& p3, const Vector3& p4)
{
	float a11, a12, a13, a21, a22, a23, a31, a32, a33, b1, b2, b3, d, d1, d2, d3;
	a11 = 2 * (p2.x - p1.x); a12 = 2 * (p2.y - p1.y); a13 = 2 * (p2.z - p1.z);
	a21 = 2 * (p3.x - p2.x); a22 = 2 * (p3.y - p2.y); a23 = 2 * (p3.z - p2.z);
	a31 = 2 * (p4.x - p3.x); a32 = 2 * (p4.y - p3.y); a33 = 2 * (p4.z - p3.z);
	b1 = p2.x * p2.x - p1.x * p1.x + p2.y * p2.y - p1.y * p1.y + p2.z * p2.z - p1.z * p1.z;
	b2 = p3.x * p3.x - p2.x * p2.x + p3.y * p3.y - p2.y * p2.y + p3.z * p3.z - p2.z * p2.z;
	b3 = p4.x * p4.x - p3.x * p3.x + p4.y * p4.y - p3.y * p3.y + p4.z * p4.z - p3.z * p3.z;
	d = a11 * a22 * a33 + a12 * a23 * a31 + a13 * a21 * a32 - a11 * a23 * a32 - a12 * a21 * a33 - a13 * a22 * a31;
	d1 = b1 * a22 * a33 + a12 * a23 * b3 + a13 * b2 * a32 - b1 * a23 * a32 - a12 * b2 * a33 - a13 * a22 * b3;
	d2 = a11 * b2 * a33 + b1 * a23 * a31 + a13 * a21 * b3 - a11 * a23 * b3 - b1 * a21 * a33 - a13 * b2 * a31;
	d3 = a11 * a22 * b3 + a12 * b2 * a31 + b1 * a21 * a32 - a11 * b2 * a32 - a12 * a21 * b3 - b1 * a22 * a31;

	return Vector3(d1 / d, d2 / d, d3 / d);
}
