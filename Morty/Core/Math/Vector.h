#ifndef _VECTOR_H_
#define _VECTOR_H_

#include "Vector_generated.h"
#include "Utility/MGlobal.h"
#include "Math/Matrix.h"

class Vector3;
class Vector4;
class Vector2i;

class Matrix3;
class Matrix4;

class MORTY_API Vector2
{
public:
	Vector2();
	Vector2(const float& _x, const float& _y);
	Vector2(const Vector2& vec2) = default;
	Vector2(const Vector3& vec3);
	explicit Vector2(const Vector2i& vec2);

	float Length() const;
	void Normalize();

	Vector2 operator - (void) const;

	Vector2 operator + (const Vector2& value) const;
	Vector2 operator - (const Vector2& value) const;

	Vector2 operator * (const float& value) const;
	Vector2 operator / (const float& value) const;

	Vector2& operator = (const Vector2& value) = default;
	Vector3 operator = (const Vector3& value);
	Vector4 operator = (const Vector4& value);

	float operator * (const Vector2& value) const;
	float CrossProduct(const Vector2& value) const;

	bool operator == (const Vector2& value) const;
	bool operator != (const Vector2& value) const;

	const Vector2& operator *= (const float& value);
	const Vector2& operator /= (const float& value);

public:

	const mfbs::Vector2* Serialize(flatbuffers::FlatBufferBuilder& fbb) const;
	void Deserialize(const void* pBufferPointer);

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
	Vector3(const float& value);
	Vector3(const Vector2& vec2);
	Vector3(const Vector2& vec2, const float& z);
	Vector3(const Vector3& vec3);
	Vector3(const Vector4& vec4);
	Vector3(const float& x, const float& y, const float& z);
	Vector3(const mfbs::Vector3& value);

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

	static Vector3 Fill(float value) { return Vector3(value, value, value); }

public:

	const mfbs::Vector3* Serialize(flatbuffers::FlatBufferBuilder& fbb) const;
	void Deserialize(const void* pBufferPointer);

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
	static const Vector3 One;
	static const Vector3 Right;
	static const Vector3 Left;
	static const Vector3 Up;
	static const Vector3 Down;
	static const Vector3 Forward;
	static const Vector3 Backward;

};

class MORTY_API Vector2i
{
public:
	Vector2i() = default;
	Vector2i(int x, int y);
public:
	union
	{
		struct
		{
			int x;
			int y;
		};
		int m[2];
	};

	bool operator!=(const Vector2i& other) const;
	bool operator==(const Vector2i& other) const;
};

class MORTY_API Vector3i
{
public:
	Vector3i(int x, int y, int z);

	Vector3i operator* (const int& value) const;

public:
	union
	{
		struct
		{
			int x;
			int y;
			int z;
		};
		int m[3];
	};
};

Vector3 operator* (const float& value, const Vector3& vector);


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

    Vector3& GetVector3() const;
    
	float Length() const;
	void Normalize();

public:

	const mfbs::Vector4* Serialize(flatbuffers::FlatBufferBuilder& fbb) const;
	void Deserialize(const void* pBufferPointer);

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
