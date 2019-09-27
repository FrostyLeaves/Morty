#include "MTransform.h"

MTransform::MTransform()
	: m_v3Position(0, 0, 0)
	, m_v3Scale(1, 1, 1)
	, m_qtRotation()
{

}

MTransform::~MTransform()
{

}

Matrix4 MTransform::GetMatrix()
{
	Matrix4 matmove = Matrix4::IdentityMatrix;
	matmove.m[3][0] = m_v3Position.x;
	matmove.m[3][1] = m_v3Position.y;
	matmove.m[3][2] = m_v3Position.z;

	Matrix4 matrota = m_qtRotation.GetMatrix();

	Matrix4 matscal = Matrix4::IdentityMatrix;
	matscal.m[0][0] = m_v3Scale.x;
	matscal.m[1][1] = m_v3Scale.y;
	matscal.m[2][2] = m_v3Scale.z;

	return matscal * matrota * matmove;
}
