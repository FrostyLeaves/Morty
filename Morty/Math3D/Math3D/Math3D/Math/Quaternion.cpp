#include "Quaternion.h"
#include <cmath>
#include <cassert>

Quaternion::Quaternion()
	: w(1)
	, x(0)
	, y(0)
	, z(0)
	
{

}

Quaternion::Quaternion(const float& w, const float& x, const float& y, const float& z)
	: w(w)
	, x(x)
	, y(y)
	, z(z)
{

}

Quaternion::Quaternion(const float& w, const Vector3& v)
	: w(w)
	, x(v.x)
	, y(v.y)
	, z(v.z)
{

}

Quaternion::Quaternion(const Vector3& vAxis, const float& fAngle)
	: w(0)
	, x(0)
	, y(0)
	, z(0)
{
	RotateAxis(vAxis, fAngle);
}

Quaternion Quaternion::operator-(void) const
{
	return Quaternion(-w, -x, -y, -z);
}

Quaternion Quaternion::CrossProduct(const Quaternion& quat) const
{
	return Quaternion(	w * quat.w - x * quat.x - y * quat.y - z * quat.z,
						w * quat.x + x * quat.w + z * quat.y - y * quat.z,
						w * quat.y + y * quat.w + x * quat.z - z * quat.x,
						w * quat.z + z * quat.w + y * quat.x - x * quat.y);
}

Quaternion Quaternion::operator*(const Quaternion& quat) const
{
	return CrossProduct(quat);
}

float Quaternion::DotProduct(const Quaternion& quat) const
{
	return w * quat.w + x * quat.x + y * quat.y + z * quat.z;
}

void Quaternion::RotateX(const float& fAngle)
{
	float angleHalf = fAngle * 0.5f;

	w = cos(angleHalf);
	x = sin(angleHalf);
	y = 0;
	z = 0;
}

void Quaternion::RotateY(const float& fAngle)
{
	float angleHalf = fAngle * 0.5f;

	w = cos(angleHalf);
	x = 0;
	y = sin(angleHalf);
	z = 0;
}

void Quaternion::RotateZ(const float& fAngle)
{
	float angleHalf = fAngle * 0.5f;

	w = cos(angleHalf);
	x = 0;
	y = 0;
	z = sin(angleHalf);
}

void Quaternion::RotateAxis(Vector3 axis, const float& fAngle)
{
	axis.Normalization();
	float angleHalf = fAngle * 0.5f;

	w = cos(angleHalf);
	x = axis.x * sin(angleHalf);
	y = axis.y * sin(angleHalf);
	z = axis.z * sin(angleHalf);
}

float Quaternion::Length() const
{
	return sqrtf(w * w + x * x + y * y + z * z);
}

void Quaternion::Normalize()
{
	float length = Length();
	assert(length > 0.0f);

	w = w / length;
	x = x / length;
	y = y / length;
	z = z / length;
}

float Quaternion::GetAngle()
{
	return acos(w) * 2.0f;
}

Vector3 Quaternion::GetAxis()
{
	float sinHalf = sin(acos(w));

	if (sinHalf <= 0.0f)
		return Vector3(1.0f, 0.0f, 0.0f);

	Vector3 ret(x / sinHalf, y / sinHalf, z / sinHalf);
	ret.Normalization();

	return ret;

}

Matrix4 Quaternion::GetMatrix()
{
	float xx = x * x;
	float yy = y * y;
	float zz = z * z;
	float xy = x * y;
	float xz = x * z;
	float yz = y * z;
	float wx = w * x;
	float wy = w * y;
	float wz = w * z;

	return Matrix4(1 - 2 * (yy + zz), 2 * (xy + wz), 2 * (xz - wy), 0,
		2 * (xy - wz), 1 - 2 * (xx + zz), 2 * (yz + wx), 0,
		2 * (xz + wy), 2 * (yz - wx), 1 - 2 * (xx + yy), 0,
		0, 0, 0, 1);
}

Quaternion Quaternion::Slerp(const Quaternion& quat1, Quaternion quat2, const float fSmooth)
{
	if (fSmooth <= 0.0f)
		return quat1;
	if (fSmooth >= 1.0f)
		return quat2;

	float cosOmega = quat1.DotProduct(quat2);

	float k1 = 1.0f - fSmooth;
	float k2 = fSmooth;

	if (cosOmega < 0.0f)
	{
		quat2.w = -quat2.w;
		quat2.x = -quat2.x;
		quat2.y = -quat2.y;
		quat2.z = -quat2.z;
	}

	if (cosOmega > 0.9999f)
	{
		float sinOmega = 1 - cosOmega * cosOmega;
		float omega = atan2(sinOmega, cosOmega);

		k1 = sin((1.0f - fSmooth) * omega) / sinOmega;
		k2 = sin(fSmooth) / sinOmega;
	}

	return Quaternion(	quat1.w * k1 + quat2.w * k2,
						quat1.x * k1 + quat2.x * k2,
						quat1.y * k1 + quat2.y * k2,
						quat1.z * k1 + quat2.z * k2);

}
