#ifndef _VECTOR_H_
#define _VECTOR_H_

#ifndef MATH3D_EXPORTS
#define MATH_IOE_DLL _declspec(dllimport)
#else
#define MATH_IOE_DLL _declspec(dllexport)
#endif

class Vector3;
class Vector4;

class MATH_IOE_DLL Vector2
{
public:
	Vector2();
	Vector2(const float& _x, const float& _y);
	Vector2(const Vector3& vec3);

public:
	union
	{
		struct
		{
			float x;
			float y;
		};
		float m[2];
	};
};

class MATH_IOE_DLL Vector3
{
public:
	Vector3();
	Vector3(const Vector3& vec3);
	Vector3(const float& x, const float& y, const float& z);

	float Length() const;
	void Normalization();

	Vector3 operator - (void) const;

	Vector3 operator * (const float& value) const;
	Vector3 operator / (const float& value) const;

	Vector3 operator = (const Vector3& value);
	Vector3 operator = (const Vector4& value);

	const Vector3& operator *= (const float& value);
	const Vector3& operator /= (const float& value);

	Vector3 operator + (const Vector3& value) const;
	Vector3 operator - (const Vector3& value) const;

	const Vector3& operator += (const Vector3& value);
	const Vector3& operator -= (const Vector3& value);

	float operator * (const Vector3& value) const;

	Vector3 CrossProduct(const Vector3& value) const;

	bool operator == (const Vector3& value) const;

public:
	union
	{
		struct
		{
			float x;
			float y;
			float z;
		};
		float m[3];
	};
};

Vector3 operator * (const float& value, const Vector3& vector);

const Vector3 VECTOR3_ZERO(0.0f, 0.0f, 0.0f);




class MATH_IOE_DLL Vector4
{
public:
	Vector4();
	Vector4(const float& _x, const float& _y, const float& _z, const float& _w);
	Vector4(const Vector3& vec3, const float& _w);

	Vector4 operator * (const float& value) const;
	Vector4 operator / (const float& value) const;

public:
	union
	{
		struct 
		{
			float x;
			float y;
			float z;
			float w;
		};
		float m[4];
	};
};

#endif