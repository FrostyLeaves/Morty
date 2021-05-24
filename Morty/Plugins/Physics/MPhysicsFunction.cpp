#include "MPhysicsFunction.h"

#include "btBulletDynamicsCommon.h"

MPhysicsFunction::MPhysicsFunction()
{

}

MPhysicsFunction::~MPhysicsFunction()
{

}

void MPhysicsFunction::ConvertTransform(const MTransform& transform0, btTransform* transform1)
{
	if (!transform1)
		return;

	btVector3 origin;
	ConvertVector3(transform0.GetPosition(), &origin);
	transform1->setOrigin(origin);

	btMatrix3x3 basis;
	Matrix4 mat4 = transform0.GetMatrix();
	Matrix3 mat3(mat4, 3, 3);
	ConvertMatrix3(mat3, &basis);
	transform1->setBasis(basis);
}

void MPhysicsFunction::ConvertTransform(const btTransform* transform0, MTransform& transform1)
{
	if (!transform0)
		return;

	btVector3 origin0 = transform0->getOrigin();
	Vector3 origin1;
	ConvertVector3(&origin0, origin1);
	transform1.SetPosition(origin1);

	btQuaternion rotation0 = transform0->getRotation();
	Quaternion rotation1;
	ConvertQuaternion(&rotation0, rotation1);
	transform1.SetRotation(rotation1);
}

void MPhysicsFunction::ConvertTransform(const Matrix4& transform0, btTransform* transform1)
{
	if (!transform1)
		return;

	btVector3 origin;
	ConvertVector3(transform0.GetTranslation(), &origin);
	transform1->setOrigin(origin);

	btMatrix3x3 basis;
	Matrix3 mat3(transform0, 3, 3);
	ConvertMatrix3(mat3, &basis);
	transform1->setBasis(basis);
}

void MPhysicsFunction::ConvertVector3(const Vector3& vector0, btVector3* vector1)
{
	if (!vector1)
		return;

	vector1->setX(vector0.x);
	vector1->setY(vector0.y);
	vector1->setZ(vector0.z);
}

void MPhysicsFunction::ConvertVector3(const btVector3* vector0, Vector3& vector1)
{
	if (!vector0)
		return;

	vector1.x = vector0->x();
	vector1.y = vector0->y();
	vector1.z = vector0->z();
}

void MPhysicsFunction::ConvertMatrix3(const Matrix3& matrix0, btMatrix3x3* matrix1)
{
	if (!matrix1)
		return;

	for (int x = 0; x < 3; ++x)
	{
		for (int y = 0; y < 3; ++y)
		{
			(*matrix1)[x][y] = matrix0.m[x][y];
		}
	}
}

void MPhysicsFunction::ConvertMatrix3(const btMatrix3x3* matrix0, Matrix3& matrix1)
{
	if (!matrix0)
		return;

	for (int x = 0; x < 3; ++x)
	{
		for (int y = 0; y < 3; ++y)
		{
			matrix1.m[x][y] = (*matrix0)[x][y];
		}
	}
}

void MPhysicsFunction::ConvertQuaternion(const btQuaternion* quat0, Quaternion& quat1)
{
	if (!quat0)
		return;

	quat1.x = quat0->x();
	quat1.y = quat0->y();
	quat1.z = quat0->z();
	quat1.w = quat0->w();
}
