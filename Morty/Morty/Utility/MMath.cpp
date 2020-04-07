#include "MMath.h"
#include "Timer/MTimer.h"

std::default_random_engine MMath::s_randomEngine(MTimer::GetCurTime());

Matrix4 MMath::GetScaleAndRotation(const Matrix4& mat)
{
	Matrix4 result = mat;
	result.m[0][3] = 0;
	result.m[1][3] = 0;
	result.m[2][3] = 0;

	return result;
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
