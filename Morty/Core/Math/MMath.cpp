#include "MMath.h"
#include "MTimer.h"

std::default_random_engine MMath::s_randomEngine(MTimer::GetCurTime());

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
