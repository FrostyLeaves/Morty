#include "MBounds.h"
#include "Dense"
#include "Eigenvalues"
#include "Matrix.h"

MBoundsOBB::MBoundsOBB(const std::vector<Vector3>& vPoints)
{
	unsigned int unVertCount = vPoints.size();
	Vector3 v3Average;

	float cov_xx = 0;
	float cov_yy = 0;
	float cov_zz = 0;
	float cov_xy = 0;
	float cov_xz = 0;
	float cov_yz = 0;

	for (const Vector3 pos : vPoints)
	{
		v3Average += pos;
	}

	v3Average /= unVertCount;

	for (const Vector3& pos : vPoints)
	{
		cov_xx += (pos.x - v3Average.x) * (pos.x - v3Average.x);
		cov_yy += (pos.y - v3Average.y) * (pos.y - v3Average.y);
		cov_zz += (pos.z - v3Average.z) * (pos.z - v3Average.z);
		cov_xy += (pos.x - v3Average.x) * (pos.y - v3Average.y);
		cov_xz += (pos.x - v3Average.x) * (pos.z - v3Average.z);
		cov_yz += (pos.y - v3Average.y) * (pos.z - v3Average.z);
	}
	
	cov_xx /= (unVertCount);
	cov_yy /= (unVertCount);
	cov_zz /= (unVertCount);
	cov_xy /= (unVertCount);
	cov_xz /= (unVertCount);
	cov_yz /= (unVertCount);


	Eigen::Matrix3d matCov;
	matCov << cov_xx, cov_xy, cov_xz, cov_xy, cov_yy, cov_yz, cov_xz, cov_yz, cov_zz;
	Eigen::EigenSolver<Eigen::Matrix3d> es(matCov);

	Eigen::Matrix3d matTemp = es.pseudoEigenvectors();
	Eigen::Matrix3d matEigVectors = matTemp.transpose();

/*
	matEigVectors.col(0).normalize();
	double temp;
	for (std::size_t k = 0; k != matEigVectors.cols() - 1; ++k)
	{
		for (std::size_t j = 0; j != k + 1; ++j)
		{
			temp = matEigVectors.col(j).transpose() * matEigVectors.col(k + 1);
			matEigVectors.col(k + 1) -= matEigVectors.col(j) * temp;
		}
		matEigVectors.col(k + 1).normalize();
	}
*/

	for (int i = 0; i < 3; ++i)
	{
		for (int j = 0; j < 3; ++j)
		{
			m_matEigVectors.m[i][j] = *(matEigVectors.data() + i * 3 + j);
		}
	}

	Vector3 v3MinPoint, v3MaxPoint;
	for (const Vector3& pos : vPoints)
	{
		v3MinPoint = v3MaxPoint = pos * m_matEigVectors;
		break;
	}

	for (const Vector3& pos : vPoints)
	{
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
