#include "Matrix.h"


Matrix4::Matrix4()
{
	for (int i= 0; i < 4; ++i)
		for (int j = 0; j < 4; ++j)
			m[i][j] = 0.0f;
}

Matrix4::Matrix4(float value[4][4])
{
	for (int i = 0; i < 4; ++i)
		for (int j = 0; j < 4; ++j)
			m[i][j] = value[i][j];
}

Matrix4::Matrix4(const float& m_00, const float& m_01, const float& m_02, const float& m_03, const float& m_10, const float& m_11, const float& m_12, const float& m_13, const float& m_20, const float& m_21, const float& m_22, const float& m_23, const float& m_30, const float& m_31, const float& m_32, const float& m_33)
{
	m[0][0] = m_00;
	m[0][1] = m_01;
	m[0][2] = m_02;
	m[0][3] = m_03;

	m[1][0] = m_10;
	m[1][1] = m_11;
	m[1][2] = m_12;
	m[1][3] = m_13;

	m[2][0] = m_20;
	m[2][1] = m_21;
	m[2][2] = m_22;
	m[2][3] = m_23;

	m[3][0] = m_30;
	m[3][1] = m_31;
	m[3][2] = m_32;
	m[3][3] = m_33;
}

Matrix4 Matrix4::Transposed() const
{
	Matrix4 mat(m[0][0], m[1][0], m[2][0], m[3][0],
		m[0][1], m[1][1], m[2][1], m[3][1],
		m[0][2], m[1][2], m[2][2], m[3][2],
		m[0][3], m[1][3], m[2][3], m[3][3]);

	return mat;
}

Matrix4 Matrix4::operator*(const Matrix4& mat) const
{
	Matrix4 result;
	for (int i = 0; i < 4; ++i)
		for (int j = 0; j < 4; ++j)
			for (int k = 0; k < 4; ++k)
				result.m[i][j] += m[i][k] * mat.m[k][j];

	return result;

}

Matrix4 Matrix4::operator*(const float& value) const
{
	Matrix4 result;
	for (int i = 0; i < 4; ++i)
		for (int j = 0; j < 4; ++j)
			result.m[i][j] = value * m[i][j];

	return result;
}

void Matrix4::Translation(const float& x, const float& y, const float& z)
{
	m[3][0] = x;
	m[3][1] = y;
	m[3][2] = z;
}

Matrix4 Matrix4::operator/(const float& value) const
{
	return *this * (1.0f / value);
}

bool Matrix4::operator==(const Matrix4& mat) const
{
	for (int i = 0; i < 4; ++i)
		for (int j = 0; j < 4; ++j)
		{
			if (mat.m[i][j] != m[i][j])
				return false;
		}

	return true;
}

float Matrix3::Determinant() const
{
	return m[0][0] * (m[1][1] * m[2][2] - m[1][2] * m[2][1])
		+ m[0][1] * (m[1][2] * m[2][0] - m[1][0] * m[2][2])
		+ m[0][2] * (m[1][0] * m[2][1] - m[1][1] * m[2][0]);
}

Matrix3::Matrix3(const Matrix4& mat4, const int& di, const int& dj)
{
	int i3 = 0;
	int i = 0;
	while (i < 4)
	{
		if (i == di)
			++i;

		int j = 0;
		int j3 = 0;
		while (j < 4)
		{
			if (j == dj)
				++j;

			m[i3][j3] = mat4.m[i][j];
			++j3;
			++j;
		}

		++i3;
		++i;
	}
}

float Matrix4::Determinant() const
{
	float a = m[0][0] * (m[1][1] * (m[2][2] * m[3][3] - m[2][3] * m[3][2]) + m[1][2] * (m[2][3] * m[3][1] - m[2][1] * m[3][3]) + m[1][3] * (m[2][1] * m[3][2] - m[2][2] * m[3][1]));
	float b = -m[0][1] * (m[1][0] * (m[2][2] * m[3][3] - m[2][3] * m[3][2]) + m[1][2] * (m[2][3] * m[3][0] - m[2][0] * m[3][3]) + m[1][3] * (m[2][0] * m[3][2] - m[2][2] * m[3][0]));
	float c = +m[0][2] * (m[1][0] * (m[2][1] * m[3][3] - m[2][3] * m[3][1]) + m[1][1] * (m[2][3] * m[3][0] - m[2][0] * m[3][3]) + m[1][3] * (m[2][0] * m[3][1] - m[2][1] * m[3][0]));
	float d = -m[0][3] * (m[1][0] * (m[2][1] * m[3][2] - m[2][2] * m[3][1]) + m[1][1] * (m[2][2] * m[3][0] - m[2][0] * m[3][2]) + m[1][2] * (m[2][0] * m[3][1] - m[2][1] * m[3][0]));

	return m[0][0] * (m[1][1] * (m[2][2] * m[3][3] - m[2][3] * m[3][2]) + m[1][2] * (m[2][3] * m[3][1] - m[2][1] * m[3][3]) + m[1][3] * (m[2][1] * m[3][2] - m[2][2] * m[3][1]))
		- m[0][1] * (m[1][0] * (m[2][2] * m[3][3] - m[2][3] * m[3][2]) + m[1][2] * (m[2][3] * m[3][0] - m[2][0] * m[3][3]) + m[1][3] * (m[2][0] * m[3][2] - m[2][2] * m[3][0]))
		+ m[0][2] * (m[1][0] * (m[2][1] * m[3][3] - m[2][3] * m[3][1]) + m[1][1] * (m[2][3] * m[3][0] - m[2][0] * m[3][3]) + m[1][3] * (m[2][0] * m[3][1] - m[2][1] * m[3][0]))
		- m[0][3] * (m[1][0] * (m[2][1] * m[3][2] - m[2][2] * m[3][1]) + m[1][1] * (m[2][2] * m[3][0] - m[2][0] * m[3][2]) + m[1][2] * (m[2][0] * m[3][1] - m[2][1] * m[3][0]));
}

Matrix4 Matrix4::CofactorMatrix() const
{
	return Matrix4(AlgebraicCofactor(0, 0), AlgebraicCofactor(0, 1), AlgebraicCofactor(0, 2), AlgebraicCofactor(0, 3), 
		AlgebraicCofactor(1, 0), AlgebraicCofactor(1, 1), AlgebraicCofactor(1, 2), AlgebraicCofactor(1, 3), 
		AlgebraicCofactor(2, 0), AlgebraicCofactor(2, 1), AlgebraicCofactor(2, 2), AlgebraicCofactor(2, 3), 
		AlgebraicCofactor(3, 0), AlgebraicCofactor(3, 1), AlgebraicCofactor(3, 2), AlgebraicCofactor(3, 3));
}

Matrix4 Matrix4::AdjointMatrix() const
{
	return CofactorMatrix().Transposed();
}

float Matrix4::AlgebraicCofactor(const int& i, const int& j) const
{
	if ((i + j) % 2 == 0)
		return Matrix3(*this, i, j).Determinant();
	return - Matrix3(*this, i, j).Determinant();
}

Matrix4 Matrix4::Inverse() const
{
	return AdjointMatrix() / Determinant();
}

bool Matrix4::IsOrthogonal() const
{
	return IdentityMatrix == *this * this->Transposed();
}

Vector3 operator*(const Vector3& vec3, const Matrix4& mat4)
{
	return Vector3(vec3.x * mat4.m[0][0] + vec3.y * mat4.m[1][0] + vec3.z * mat4.m[2][0], vec3.x * mat4.m[0][1] + vec3.y * mat4.m[1][1] + vec3.z * mat4.m[2][1], vec3.x * mat4.m[0][2] + vec3.y * mat4.m[1][2] + vec3.z * mat4.m[2][2]);
}
