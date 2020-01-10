#include "MBounds.h"
#include "Dense"
#include "Eigenvalues"
#include "Matrix.h"

MBoundsOBB::MBoundsOBB(const Vector3* vPoints, const unsigned int& unArrayLength)
{
	SetPoints((MByte*)vPoints, unArrayLength, 0, sizeof(Vector3));
}

void MBoundsOBB::SetPoints(const MByte* vPoints, const unsigned int& unArrayLength, const unsigned int& unOffset, const unsigned int& unDataSize)
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
	for(unsigned int i = 0; i < unArrayLength; ++i, pointer += unDataSize)
	{
		const Vector3& pos = *(Vector3*)(vPoints + unOffset);
		v3Average += pos;
	}

	v3Average /= unArrayLength;

	pointer = vPoints;
	for (unsigned int i = 0; i < unArrayLength; ++i, pointer += unDataSize)
	{
		const Vector3& pos = *(Vector3*)(vPoints + unOffset);
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
	for (unsigned int i = 0; i < unArrayLength; ++i, pointer += unDataSize)
	{
		const Vector3& pos = *(Vector3*)(vPoints + unOffset);
		v3MinPoint = v3MaxPoint = pos * m_matEigVectors;
		break;
	}

	pointer = vPoints;
	for (unsigned int i = 0; i < unArrayLength; ++i, pointer += unDataSize)
	{
		const Vector3& pos = *(Vector3*)(vPoints + unOffset);
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

Vector3 MBoundsOBB::ConvertToOBB(const Vector3& v3Pos) const
{
	return v3Pos * m_matEigVectors;
}

Vector3 MBoundsOBB::ConvertFromOBB(const Vector3& v3Pos) const
{
	return v3Pos * m_matEigVectors.Inverse();
}

MBoundsAABB::MBoundsAABB(const std::vector<Vector3>& vPoints)
	: m_v3MaxPoint(-FLT_MAX, -FLT_MAX, -FLT_MAX)
	, m_v3MinPoint(FLT_MAX, FLT_MAX, FLT_MAX)
{
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

MBoundsAABB::MBoundsAABB(const Matrix4& matWorld, const MBoundsOBB& obb)
{
	Vector3 v3ModelCenter = matWorld * obb.ConvertFromOBB(obb.m_v3CenterPoint);
	Vector3 v3ModelHalf = matWorld * obb.ConvertFromOBB(obb.m_v3HalfLength);
	v3ModelHalf = Vector3(fabs(v3ModelHalf.x), fabs(v3ModelHalf.y), fabs(v3ModelHalf.z));

	if (m_v3MinPoint.x > v3ModelCenter.x - v3ModelHalf.x)
		m_v3MinPoint.x = v3ModelCenter.x - v3ModelHalf.x;
	if (m_v3MinPoint.y > v3ModelCenter.y - v3ModelHalf.y)
		m_v3MinPoint.y = v3ModelCenter.y - v3ModelHalf.y;
	if (m_v3MinPoint.z > v3ModelCenter.z - v3ModelHalf.z)
		m_v3MinPoint.z = v3ModelCenter.z - v3ModelHalf.z;
	if (m_v3MaxPoint.x < v3ModelCenter.x + v3ModelHalf.x)
		m_v3MaxPoint.x = v3ModelCenter.x + v3ModelHalf.x;
	if (m_v3MaxPoint.y < v3ModelCenter.y + v3ModelHalf.y)
		m_v3MaxPoint.y = v3ModelCenter.y + v3ModelHalf.y;
	if (m_v3MaxPoint.z < v3ModelCenter.z + v3ModelHalf.z)
		m_v3MaxPoint.z = v3ModelCenter.z + v3ModelHalf.z;

	m_v3CenterPoint = (m_v3MinPoint + m_v3MaxPoint) * 0.5f;
	m_v3HalfLength = (m_v3MaxPoint - m_v3MinPoint) * 0.5f;
}

void MBoundsAABB::GetPoints(std::vector<Vector3>& vPoints)
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

void MBoundsAABB::UnionMinMax(Vector3& v3Min, Vector3& v3Max)
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
