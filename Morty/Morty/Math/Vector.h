#ifndef _VECTOR_H_
#define _VECTOR_H_

#include "MGlobal.h"

#include "Matrix.h"

class Vector3;
class Vector4;

class Matrix3;
class Matrix4;

class MORTY_API Vector2
{
public:
	Vector2();
	Vector2(const float& _x, const float& _y);
	Vector2(const Vector3& vec3);

	float Length() const;
	void Normalize();

	Vector2 operator - (void) const;

	Vector2 operator + (const Vector2& value) const;
	Vector2 operator - (const Vector2& value) const;

	Vector2 operator * (const float& value) const;
	Vector2 operator / (const float& value) const;

	Vector2 operator = (const Vector2& value);
	Vector3 operator = (const Vector3& value);
	Vector4 operator = (const Vector4& value);

	float operator * (const Vector2& value) const;
	float CrossProduct(const Vector2& value) const;

	bool operator == (const Vector2& value) const;
	bool operator != (const Vector2& value) const;

	const Vector2& operator *= (const float& value);
	const Vector2& operator /= (const float& value);
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

	static const Vector2 Zero;
};

class MORTY_API Vector3
{
public:
	Vector3();
	Vector3(const Vector2& vec2);
	Vector3(const Vector2& vec2, const float& z);
	Vector3(const Vector3& vec3);
	Vector3(const Vector4& vec4);
	Vector3(const float& x, const float& y, const float& z);

	float Length() const;
	void Normalize();

	Vector3 operator - (void) const;

	Vector3 operator * (const float& value) const;
	Vector3 operator / (const float& value) const;

	Vector3 operator = (const Vector3& value);
	Vector4 operator = (const Vector4& value);

	const Vector3& operator *= (const float& value);
	const Vector3& operator /= (const float& value);

	Vector3 operator + (const Vector3& value) const;
	Vector3 operator - (const Vector3& value) const;

	const Vector3& operator += (const Vector3& value);
	const Vector3& operator -= (const Vector3& value);

	float operator * (const Vector3& value) const;

	Vector3 operator* (const Matrix3& mat3) const;
	Vector3 operator* (const Matrix4& mat4) const;

	Vector3 CrossProduct(const Vector3& value) const;

	Vector3 Projection(const Vector3& value) const;

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


	static const Vector3 Zero;
	static const Vector3 Right;
	static const Vector3 Left;
	static const Vector3 Up;
	static const Vector3 Down;
	static const Vector3 Forward;
	static const Vector3 Backward;

};

Vector3 operator * (const float& value, const Vector3& vector);


typedef Vector3 ColorRGB;



class MORTY_API Vector4
{
public:
	Vector4();
	Vector4(const float& _x, const float& _y, const float& _z, const float& _w);
	Vector4(const Vector3& vec3, const float& _w);

	Vector4 operator - (void) const;

	Vector4 operator * (const float& value) const;
	Vector4 operator / (const float& value) const;

	Vector4 operator* (const Matrix4& mat4) const;

	Vector4 operator + (const Vector4& value) const;
	Vector4 operator - (const Vector4& value) const;

	const Vector4& operator *= (const float& value);
	const Vector4& operator /= (const float& value);

    Vector3& GetVector3();
    
	float Length() const;
	void Normalize();

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
