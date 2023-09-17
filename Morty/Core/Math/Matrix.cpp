#include "Math/Matrix.h"

#include "Matrix_generated.h"

const Matrix4 Matrix4::IdentityMatrix(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);

Matrix4::Matrix4()
{
	memset(m, 0, sizeof(m));
}

Matrix4::Matrix4(float value[4][4])
{
	memcpy(m, value, sizeof(m));
}

Matrix4::Matrix4(float value[16])
{
	memcpy(m, value, sizeof(m));
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

Matrix4::Matrix4(const Quaternion& quat)
{
	float xx = quat.x * quat.x;
	float yy = quat.y * quat.y;
	float zz = quat.z * quat.z;
	float xy = quat.x * quat.y;
	float xz = quat.x * quat.z;
	float yz = quat.y * quat.z;
	float wx = quat.w * quat.x;
	float wy = quat.w * quat.y;
	float wz = quat.w * quat.z;

#if ROW_MAJOR == MATRIX_MAJOR
	m[0][0] = 1 - 2 * (yy + zz);
	m[0][1] = 2 * (xy + wz);
	m[0][2] = 2 * (xz - wy);
	m[0][3] = 0;

	m[1][0] = 2 * (xy - wz);
	m[1][1] = 1 - 2 * (xx + zz);
	m[1][2] = 2 * (yz + wx);
	m[1][3] = 0;

	m[2][0] = 2 * (xz + wy);
	m[2][1] = 2 * (yz - wx);
	m[2][2] = 1 - 2 * (xx + yy);
	m[2][3] = 0;

	m[3][0] = 0;
	m[3][1] = 0;
	m[3][2] = 0;
	m[3][3] = 1;
#else
	m[0][0] = 1 - 2 * (yy + zz);
	m[1][0] = 2 * (xy + wz);
	m[2][0] = 2 * (xz - wy);
	m[3][0] = 0;

	m[0][1] = 2 * (xy - wz);
	m[1][1] = 1 - 2 * (xx + zz);
	m[2][1] = 2 * (yz + wx);
	m[3][1] = 0;

	m[0][2] = 2 * (xz + wy);
	m[1][2] = 2 * (yz - wx);
	m[2][2] = 1 - 2 * (xx + yy);
	m[3][2] = 0;

	m[0][3] = 0;
	m[1][3] = 0;
	m[2][3] = 0;
	m[3][3] = 1;
#endif
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
		for (int k = 0; k < 4; ++k)
			for (int j = 0; j < 4; ++j)
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

Vector4 Matrix4::operator*(const Vector4& value) const
{
	Vector4 result;
	for (int i = 0; i < 4; ++i)
		for (int j = 0; j < 4; ++j)
			result.m[i] += m[i][j] * value.m[j];

	return result;
}

Vector3 Matrix4::operator*(const Vector3& value) const
{
	Vector3 result;//w == 1
	for (int i = 0; i < 3; ++i)
	{
		for (int j = 0; j < 3; ++j)
			result.m[i] += m[i][j] * value.m[j];
		result.m[i] += m[i][3];
	}
	return result;
}

Vector4 Matrix4::Row(const unsigned int& row) const
{
	return Vector4(m[row][0], m[row][1], m[row][2], m[row][3]);
}

Vector4 Matrix4::Col(const unsigned int& col) const
{
	return Vector4(m[0][col], m[1][col], m[2][col], m[3][col]);
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

Matrix3 Matrix3::CofactorMatrix() const
{
	return Matrix3(AlgebraicCofactor(0, 0), AlgebraicCofactor(0, 1), AlgebraicCofactor(0, 2), 
		AlgebraicCofactor(1, 0), AlgebraicCofactor(1, 1), AlgebraicCofactor(1, 2), 
		AlgebraicCofactor(2, 0), AlgebraicCofactor(2, 1), AlgebraicCofactor(2, 2));
}

Matrix3 Matrix3::AdjointMatrix() const
{
	return CofactorMatrix().Transposed();
}

float Matrix3::AlgebraicCofactor(const int& i, const int& j) const
{
	Matrix2 mat2(*this, i, j);
	float fResult = mat2.m[0][0] * mat2.m[1][1] - mat2.m[0][1] * mat2.m[1][0];
	if ((i + j) % 2 == 0)
		return fResult;
	else
		return -fResult;
}

Matrix3 Matrix3::Inverse() const
{
	return AdjointMatrix() / Determinant();
}

Matrix3 Matrix3::operator/(const float& value) const
{
	return *this * (1.0f / value);
}

const mfbs::Matrix3* Matrix3::Serialize(flatbuffers::FlatBufferBuilder& fbb) const
{
	MORTY_UNUSED(fbb);

	return reinterpret_cast<const mfbs::Matrix3*>(this);
}

void Matrix3::Deserialize(const void* pBufferPointer)
{
	memcpy(this, pBufferPointer, sizeof(Matrix3));
}

Matrix3 Matrix3::operator*(const float& value) const
{
	Matrix3 result;
	for (int i = 0; i < 3; ++i)
		for (int j = 0; j < 3; ++j)
			result.m[i][j] = value * m[i][j];

	return result;
}

Matrix3 Matrix3::operator*(const Matrix3& mat) const
{
	Matrix3 result;
	for (int i = 0; i < 3; ++i)
		for (int k = 0; k < 3; ++k)
			for (int j = 0; j < 3; ++j)
				result.m[i][j] += m[i][k] * mat.m[k][j];

	return result;

}

Vector3 Matrix3::operator* (const Vector3& value) const
{
	Vector3 result;//w == 1
	for (int i = 0; i < 3; ++i)
	{
		for (int j = 0; j < 3; ++j)
		{
			result.m[i] += m[i][j] * value.m[j];
		}
	}
	return result;
}

Matrix3::Matrix3()
{
	memset(m, 0, sizeof(m));
}

Matrix3::Matrix3(const float* vValues)
{
	memcpy(m, vValues, 9);
}

Matrix3::Matrix3(const float& m_00, const float& m_01, const float& m_02, const float& m_10, const float& m_11, const float& m_12, const float& m_20, const float& m_21, const float& m_22)
{
	m[0][0] = m_00;
	m[0][1] = m_01;
	m[0][2] = m_02;
	m[1][0] = m_10;
	m[1][1] = m_11;
	m[1][2] = m_12;
	m[2][0] = m_20;
	m[2][1] = m_21;
	m[2][2] = m_22;
}

Matrix3::Matrix3(const Matrix4& mat4, const int& di, const int& dj)
{
	for (int i = 0; i < 3; ++i)
	{
		for (int j = 0; j < 3; ++j)
		{
			m[i][j] = mat4.m[i + (i >= di)][j + (j >= dj)];
		}
	}
}

Matrix3 Matrix3::Transposed() const
{
	return Matrix3(m[0][0], m[1][0], m[2][0],
		m[0][1], m[1][1], m[2][1],
		m[0][2], m[1][2], m[2][2]);
}

float Matrix4::Determinant() const
{
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

// 
// void Matrix4::SetTranslation(const float& x, const float& y, const float& z)
// {
// 	m[3][0] = x;
// 	m[3][1] = y;
// 	m[3][2] = z;
// }
//
// 
// Matrix4 Matrix4::GetTransPart()
// {
// 	Matrix4 mat = IdentityMatrix;
// 	mat.SetTranslation(m[3][0], m[3][1], m[3][2]);
// 	return mat;
// }
// 
 Matrix4 Matrix4::GetRotatePart() const
 {
 	float lineLength[3];

#if ROW_MAJOR == MATRIX_MAJOR
	lineLength[0] = Vector3(m[0][0], m[0][1], m[0][2]).Length();
	lineLength[1] = Vector3(m[1][0], m[1][1], m[1][2]).Length();
	lineLength[2] = Vector3(m[2][0], m[2][1], m[2][2]).Length();
#else
	lineLength[0] = Vector3(m[0][0], m[1][0], m[2][0]).Length();
	lineLength[1] = Vector3(m[0][1], m[1][1], m[2][1]).Length();
	lineLength[2] = Vector3(m[0][2], m[1][2], m[2][2]).Length();
#endif

 	Matrix4 mat = IdentityMatrix;
 
 	for (int row = 0; row < 3; ++row)
 	{
 		for (int col = 0; col < 3; ++col)
 		{
#if ROW_MAJOR == MATRIX_MAJOR
			if(lineLength[row] < -1e-6 || lineLength[row] > 1e-6)
 				mat.m[row][col] = m[row][col] / lineLength[row];
			else
				mat.m[row][col] = 0.0f;
#else
			if (lineLength[col] < -1e-6 || lineLength[col] > 1e-6)
				mat.m[row][col] = m[row][col] / lineLength[col];
			else
				mat.m[row][col] = 0.0f;
#endif
 		}
 	}
 
 	return mat;
 }

Matrix4 Matrix4::GetScalePart() const
{
	const Vector3 scale = GetScale();

	Matrix4 mat = IdentityMatrix;

	mat.m[0][0] = scale.x;
	mat.m[1][1] = scale.y;
	mat.m[2][2] = scale.z;

	return mat;
}

Vector3 Matrix4::GetScale() const
{
	Vector3 v3Scale;
#if COL_MAJOR == MATRIX_MAJOR
	v3Scale.x = Vector3(m[0][0], m[1][0], m[2][0]).Length();
	v3Scale.y = Vector3(m[0][1], m[1][1], m[2][1]).Length();
	v3Scale.z = Vector3(m[0][2], m[1][2], m[2][2]).Length();
#else
	v3Scale.x = Vector3(m[0][0], m[0][1], m[0][2]).Length();
	v3Scale.y = Vector3(m[1][0], m[1][1], m[1][2]).Length();
	v3Scale.z = Vector3(m[2][0], m[2][1], m[2][2]).Length();
#endif

	return v3Scale;
}

const mfbs::Matrix4* Matrix4::Serialize(flatbuffers::FlatBufferBuilder& fbb) const
{
	MORTY_UNUSED(fbb);
	
	return reinterpret_cast<const mfbs::Matrix4*>(this);
}

void Matrix4::Deserialize(const void* pBufferPointer)
{
	memcpy(this, pBufferPointer, sizeof(Matrix4));
}


Vector3 Matrix4::GetTranslation() const
{
#if MATRIX_MAJOR == COL_MAJOR
	return Vector3(m[0][3], m[1][3], m[2][3]);
#else
	return Vector3(m[3][0], m[3][1], m[3][2]);
#endif
}

 Quaternion Matrix4::GetRotation() const
 {
	 Quaternion result;

 	Matrix4 mat = GetRotatePart();
 
 	float vWXYZLength[4] = { mat.m[0][0] + mat.m[1][1] + mat.m[2][2],
 	mat.m[0][0] - mat.m[1][1] - mat.m[2][2],
 	mat.m[1][1] - mat.m[0][0] - mat.m[2][2],
 	mat.m[2][2] - mat.m[0][0] - mat.m[1][1] };

 	int nBiggestIndex = 0;
 	float fBiggestValue = vWXYZLength[0];
 	for (int i = 1; i < 4; ++i)
 	{
 		if (fBiggestValue < vWXYZLength[i])
 		{
 			nBiggestIndex = i;
 			fBiggestValue = vWXYZLength[i];
 		}
 	}
 
 	fBiggestValue = sqrt(fBiggestValue + 1.0f) * 0.5f;
 	float mult = 0.25f / fBiggestValue;
	
#if ROW_MAJOR == MATRIX_MAJOR
	switch (nBiggestIndex)
	{
	case 0:
		result = Quaternion(fBiggestValue, (mat.m[1][2] - mat.m[2][1]) * mult, (mat.m[2][0] - mat.m[0][2]) * mult, (mat.m[0][1] - mat.m[1][0]) * mult);
		break;
	case 1:
		result = Quaternion((mat.m[1][2] - mat.m[2][1]) * mult, fBiggestValue, (mat.m[0][1] + mat.m[1][0]) * mult, (mat.m[2][0] + mat.m[0][2]) * mult);
		break;
	case 2:
		result = Quaternion((mat.m[2][0] - mat.m[0][2]) * mult, (mat.m[0][1] + mat.m[1][0]) * mult, fBiggestValue, (mat.m[1][2] + mat.m[2][1]) * mult);
		break;
	case 3:
		result = Quaternion((mat.m[0][1] - mat.m[1][0]) * mult, (mat.m[2][0] + mat.m[0][2]) * mult, (mat.m[1][2] + mat.m[2][1]) * mult, fBiggestValue);
		break;
	}
#else
	switch (nBiggestIndex)
	{
	case 0:
		result = Quaternion(fBiggestValue, (mat.m[2][1] - mat.m[1][2]) * mult, (mat.m[0][2] - mat.m[2][0]) * mult, (mat.m[1][0] - mat.m[0][1]) * mult);
		break;
	case 1:
		result = Quaternion((mat.m[2][1] - mat.m[1][2]) * mult, fBiggestValue, (mat.m[1][0] + mat.m[0][1]) * mult, (mat.m[0][2] + mat.m[2][0]) * mult);
		break;
	case 2:
		result = Quaternion((mat.m[0][2] - mat.m[2][0]) * mult, (mat.m[1][0] + mat.m[0][1]) * mult, fBiggestValue, (mat.m[2][1] + mat.m[1][2]) * mult);
		break;
	case 3:
		result = Quaternion((mat.m[1][0] - mat.m[0][1]) * mult, (mat.m[0][2] + mat.m[2][0]) * mult, (mat.m[2][1] + mat.m[1][2]) * mult, fBiggestValue);
		break;
	}
#endif

	result.Normalize();
	return result;
 	
 }

 Matrix2::Matrix2()
 {
	 memset(m, 0, sizeof(m));
 }

 Matrix2::Matrix2(const Matrix3& mat3, const int& di, const int& dj)
 {
	 for (int i = 0; i < 2; ++i)
	 {
		 for (int j = 0; j < 2; ++j)
		 {
			 m[i][j] = mat3.m[i + (i >= di)][j + (j >= dj)];
		 }
	 }
 }
