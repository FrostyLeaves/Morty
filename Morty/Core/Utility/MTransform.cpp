#include "Utility/MTransform.h"

MTransform::MTransform()
	: m_v3Position(0, 0, 0)
	, m_v3Scale(1, 1, 1)
	, m_qtRotation()
{

}

MTransform::MTransform(const Matrix4& matTransform)
{
	m_v3Position = matTransform.GetTranslation();
	m_v3Scale = matTransform.GetScale();
	m_qtRotation = matTransform.GetRotation();
	m_qtRotation.Normalize();
}

MTransform::~MTransform()
{

}

Matrix4 MTransform::GetMatrix() const
{
	Matrix4 matmove = Matrix4::IdentityMatrix;
	matmove.m[0][3] = m_v3Position.x;
	matmove.m[1][3] = m_v3Position.y;
	matmove.m[2][3] = m_v3Position.z;

	Matrix4 matrota = m_qtRotation;

	Matrix4 matscal = Matrix4::IdentityMatrix;
	matscal.m[0][0] = m_v3Scale.x;
	matscal.m[1][1] = m_v3Scale.y;
	matscal.m[2][2] = m_v3Scale.z;

	return matmove * matrota * matscal;
}

Matrix4 MTransform::GetMatrixScale()
{
	Matrix4 matscal = Matrix4::IdentityMatrix;
	matscal.m[0][0] = m_v3Scale.x;
	matscal.m[1][1] = m_v3Scale.y;
	matscal.m[2][2] = m_v3Scale.z;

	return matscal;
}
