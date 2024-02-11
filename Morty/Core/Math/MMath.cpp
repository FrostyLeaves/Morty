#include "Math/MMath.h"
#include "Utility/MTimer.h"

using namespace morty;

std::default_random_engine MMath::s_randomEngine(static_cast<uint32_t>(MTimer::GetCurTime()));

Matrix4 MMath::GetScaleAndRotation(const Matrix4& mat)
{
	Matrix4 result = mat;
	result.m[0][3] = 0;
	result.m[1][3] = 0;
	result.m[2][3] = 0;

	return result;
}

bool MMath::RayToPlane(const Vector3& v3RayOrigin, const Vector3& v3RayDirection, const Vector3& v3PlaneOrigin, const Vector3& v3PlaneNormal, Vector3& v3HitPoint)
{
	float fLength = (v3PlaneNormal * v3PlaneOrigin - v3PlaneNormal * v3RayOrigin) / (v3PlaneNormal * v3RayDirection);

	if (-1e-6 < fLength && fLength < 1e-6)
		return false;

	v3HitPoint = v3RayOrigin + v3RayDirection * fLength;

	return true;
}

float MMath::Projection(const Vector3& v3Sour, const Vector3& v3Dest)
{
	float fLength = v3Dest.Length();
	if (fLength < 1e-6)
		return 0.0f;

	return (v3Sour * v3Dest) / fLength / fLength;
}

Matrix4 MMath::LookAt(Vector3 forward, Vector3 up)
{
	Vector3 right = up.CrossProduct(forward);
	up = forward.CrossProduct(right);

	right.Normalize();
	up.Normalize();
	forward.Normalize();

	return Matrix4(right.x, up.x, forward.x, 0,
		right.y, up.y, forward.y, 0,
		right.z, up.z, forward.z, 0,
		0, 0, 0, 1);
}

float MMath::Rand_0_1()
{
	std::uniform_real_distribution<float> randomUniform(0, 1);
	return randomUniform(s_randomEngine);
}

int MMath::RandInt(const int& nMin, const int& nMax)
{
	std::uniform_int_distribution<int> randomUniform(nMin, nMax);
	return randomUniform(s_randomEngine);
}

Vector3 MMath::ConvertToSphericalCoord(const Vector3& pos)
{
	float fLength = pos.Length();
	float theta = acosf(pos.y / fLength);
	float phi = atan2f(pos.z, pos.x);

	return Vector3(fLength, theta, phi);
}

Vector3 MMath::ConvertFormSphericalCoord(const Vector3& pos)
{
	const float& fLength = pos.x;
	const float& theta = pos.y;
	const float& phi = pos.z;

	return Vector3(fLength * sin(theta) * cos(phi),
	fLength * cos(theta),
	fLength * sin(theta) * sin(phi));
}

Vector3i MMath::Round(const Vector3 vec)
{
	return Vector3i(std::round(vec.x), std::round(vec.y), std::round(vec.z));
}


Vector3i MMath::Floor(const Vector3 vec)
{
	return Vector3i(std::floor(vec.x), std::floor(vec.y), std::floor(vec.z));
}
