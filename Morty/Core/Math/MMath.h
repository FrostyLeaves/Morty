/**
 * @File         MMath
 * 
 * @Created      2019-12-10 12:42:08
 *
 * @Author       DoubleYe
**/

#ifndef _M_MMATH_H_
#define _M_MMATH_H_
#include "Utility/MGlobal.h"
#include "Math/Matrix.h"

#include <random>

class MORTY_API MMath
{
public:

	template <typename T>
	static T Lerp(const T& a, const T& b, float fSmooth)
	{
		fSmooth = fSmooth < 0 ? 0 : fSmooth > 1.0f ? 1.0f : fSmooth;
		return a * (1.0f - fSmooth) + b * (fSmooth);
	}


	static Matrix4 GetScaleAndRotation(const Matrix4& mat);

	//���ߺ�ƽ���󽻵�
	static bool RayToPlane(const Vector3& v3RayOrigin, const Vector3& v3RayDirection, const Vector3& v3PlaneOrigin, const Vector3& v3PlaneNormal, Vector3& v3HitPoint);

	//ͶӰ
	static float Projection(const Vector3& v3Sour, const Vector3& v3Dest);

	static Matrix4 LookAt(Vector3 forward, Vector3 up);

	static float Rand_0_1();
	static int RandInt(const int& nMin, const int& nMax);

	template <typename T>
	static T Clamp(const T& value, const T& min, const T& max)
	{
		if (value < min) return min;
		if (value > max) return max;
		return value;
	}

	//x: r; y: theta; z: phi
	static Vector3 ConvertToSphericalCoord(const Vector3& pos);
	static Vector3 ConvertFormSphericalCoord(const Vector3& pos);

private:

	static std::default_random_engine s_randomEngine;

};


#endif
