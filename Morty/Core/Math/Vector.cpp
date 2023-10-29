#include "Math/Vector.h"
#include <cassert>

#include "Vector_generated.h"


Vector3::Vector3(const float& _x, const float& _y, const float& _z)
	: x(_x)
	, y(_y)
	, z(_z)
{

}

Vector3::Vector3()
	: x(0)
	, y(0)
	, z(0)
{

}

Vector3::Vector3(const Vector3& vec3)
	: x(vec3.x)
	, y(vec3.y)
	, z(vec3.z)
{

}

Vector3::Vector3(const Vector2& vec2)
	: x(vec2.x)
	, y(vec2.y)
	, z(0)
{

}

Vector3::Vector3(const float& value)
	: x(value)
	, y(value)
	, z(value)
{

}

Vector3::Vector3(const Vector2& vec2, const float& z)
	: x(vec2.x)
	, y(vec2.y)
	, z(z)
{

}

Vector3::Vector3(const Vector4& vec4)
	: x(vec4.x)
	, y(vec4.y)
	, z(vec4.z)
{

}

Vector3::Vector3(const mfbs::Vector3& value)
	: x(value.x())
	, y(value.y())
	, z(value.z())
{
	
}

float Vector3::Length() const
{
	return sqrtf(x * x + y * y + z * z);
}

void Vector3::Normalize()
{
	float length = Length();
	if (length <= 1e-6)
	{
		x = 0;
		y = 0;
		z = 0;
	}
	else
	{
		(*this) /= length;
	}
}

Vector3 Vector3::operator=(const Vector3& value)
{
	x = value.x;
	y = value.y;
	z = value.z;

	return value;
}

Vector4 Vector3::operator=(const Vector4& value)
{
	x = value.x;
	y = value.y;
	z = value.z;

	return value;
}

const Vector3& Vector3::operator*=(const float& value)
{
	*this = *this * value;

	return *this;
}

Vector3 Vector3::operator*(const Matrix3& mat3) const
{
	return Vector3(x * mat3.m[0][0] + y * mat3.m[1][0] + z * mat3.m[2][0], x * mat3.m[0][1] + y * mat3.m[1][1] + z * mat3.m[2][1], x * mat3.m[0][2] + y * mat3.m[1][2] + z * mat3.m[2][2]);
}

Vector3 Vector3::operator*(const Matrix4& mat4) const
{
	return Vector3(x * mat4.m[0][0] + y * mat4.m[1][0] + z * mat4.m[2][0] + mat4.m[3][0], x * mat4.m[0][1] + y * mat4.m[1][1] + z * mat4.m[2][1] + mat4.m[3][1], x * mat4.m[0][2] + y * mat4.m[1][2] + z * mat4.m[2][2] + mat4.m[3][2]);
}

Vector3 Vector3::operator* (const float& value) const
{
	return Vector3(x * value, y * value, z * value);
}

const Vector3& Vector3::operator/=(const float& value)
{
	*this = *this / value;
	return *this;
}

const Vector3& Vector3::operator+=(const Vector3& value)
{
	*this = *this + value;
	return *this;
}

const Vector3& Vector3::operator-=(const Vector3& value)
{
	*this = *this - value;
	return *this;
}

Vector3 Vector3::CrossProduct(const Vector3& value) const
{
	return Vector3(y * value.z - z * value.y, z * value.x - x * value.z, x * value.y - y * value.x);
}

Vector3 Vector3::Projection(const Vector3& value) const
{
	float fLength = this->Length();
	if (fLength < 1e-6)
		return Vector3(0, 0, 0);


	Vector3 dir = (*this);
	dir.Normalize();

	return dir * (*this)* value / fLength;
}

const Vector3 Vector3::Zero = Vector3(0.0f, 0.0f, 0.0f);
const Vector3 Vector3::One = Vector3(1.0f, 1.0f, 1.0f);
const Vector3 Vector3::Right = Vector3(1.0f, 0.0f, 0.0f);
const Vector3 Vector3::Left = Vector3(-1.0f, 0.0f, 0.0f);
const Vector3 Vector3::Up = Vector3(0.0f, 1.0f, 0.0f);
const Vector3 Vector3::Down = Vector3(0.0f, -1.0f, 0.0f);
const Vector3 Vector3::Forward = Vector3(0.0f, 0.0f, 1.0f);
const Vector3 Vector3::Backward(0.0f, 0.0f, -1.0f);

float Vector3::operator*(const Vector3& value) const
{
	return x * value.x + y * value.y + z * value.z;
}

Vector3 Vector3::operator-(void) const
{
	return Vector3(-x, -y, -z);
}

Vector3 Vector3::operator/(const float& value) const
{
	MORTY_ASSERT(value != 0.0f);

	return operator*(1.0f / value);
}

Vector3 Vector3::operator+(const Vector3& value) const
{
	return Vector3(x + value.x, y + value.y, z + value.z);
}

Vector3 Vector3::operator-(const Vector3& value) const
{
	return *this + (-value);
}

bool Vector3::operator==(const Vector3& value) const
{
	return x == value.x && y == value.y && z == value.z;
}

Vector3 operator*(const float& value, const Vector3& vector)
{
	return vector * value;
}

Vector4::Vector4()
	: x(0)
	, y(0)
	, z(0)
	, w(0)
{

}

Vector4::Vector4(const float& _x, const float& _y, const float& _z, const float& _w)
	: x(_x)
	, y(_y)
	, z(_z)
	, w(_w)
{

}

Vector4::Vector4(const Vector3& vec3, const float& _w)
	: x(vec3.x)
	, y(vec3.y)
	, z(vec3.z)
	, w(_w)
{

}

Vector4 Vector4::operator-(void) const
{
	return Vector4(-x, -y, -z, -w);
}

Vector4 Vector4::operator*(const float& value) const
{
	return Vector4(x * value, y * value, z * value, w * value);
}

Vector4 Vector4::operator*(const Matrix4& mat4) const
{
	return Vector4(x * mat4.m[0][0] + y * mat4.m[1][0] + z * mat4.m[2][0] + w * mat4.m[3][0], x * mat4.m[0][1] + y * mat4.m[1][1] + z * mat4.m[2][1] + w * mat4.m[3][1], x * mat4.m[0][2] + y * mat4.m[1][2] + z * mat4.m[2][2] + w * mat4.m[3][2], x * mat4.m[0][3] + y * mat4.m[1][3] + z * mat4.m[2][3] + w * mat4.m[3][3]);
}

float Vector4::Length() const
{
	return sqrtf(x * x + y * y + z * z + w * w);
}

void Vector4::Normalize()
{
	float length = Length();
	if (length <= 1e-6)
	{
		x = 0;
		y = 0;
		z = 0;
		w = 0;
	}
	else
	{
		(*this) /= length;
	}
}

const Vector4& Vector4::operator/=(const float& value)
{
	return *this = *this / value;
}

Vector4 Vector4::operator/(const float& value) const
{
	MORTY_ASSERT(value != 0.0f);

	return operator*(1.0f / value);
}

Vector4 Vector4::operator+(const Vector4& value) const
{
	return Vector4(x + value.x, y + value.y, z + value.z, w + value.w);
}

Vector4 Vector4::operator-(const Vector4& value) const
{
	return Vector4(x - value.x, y - value.y, z - value.z, w - value.w);
}

const Vector4& Vector4::operator*=(const float& value)
{
	return *this = *this * value;
}

Vector3& Vector4::GetVector3() const
{
    return *((Vector3*)(this));
}

Vector2::Vector2()
	: x(0.0f)
	, y(0.0f)
{

}

Vector2::Vector2(const float& _x, const float& _y)
	: x(_x)
	, y(_y)
{

}

Vector2::Vector2(const Vector3& vec3)
	: x(vec3.x)
	, y(vec3.y)
{

}

Vector2::Vector2(const Vector2i& vec2)
    : x(vec2.x)
    , y(vec2.y)
{
    
}

float Vector2::Length() const
{
	return sqrtf(x * x + y * y);
}

void Vector2::Normalize()
{
	float length = Length();
	if (length <= 1e-6)
	{
		x = 0;
		y = 0;
	}
	else
	{
		(*this) /= length;
	}
}

float Vector2::CrossProduct(const Vector2& value) const
{
	return x * value.y - y * value.x;
}

const Vector2 Vector2::Zero = Vector2(0.0f, 0.0f);

Vector4 Vector2::operator=(const Vector4& value)
{
	x = value.x;
	y = value.y;
	return value;
}

Vector3 Vector2::operator=(const Vector3& value)
{
	x = value.x;
	y = value.y;
	return value;
}

float Vector2::operator*(const Vector2& value) const
{
	return x * value.x + y * value.y;
}

Vector2 Vector2::operator/(const float& value) const
{
	return Vector2(x / value, y / value);
}

const Vector2& Vector2::operator/=(const float& value)
{
	*this = *this / value;
	return *this;
}

const Vector2& Vector2::operator*=(const float& value)
{
	*this = *this * value;
	return *this;
}

bool Vector2::operator==(const Vector2& value) const
{
	return x == value.x && y == value.y;
}

bool Vector2::operator!=(const Vector2& value) const
{
	return x != value.x || y != value.y;
}

Vector2 Vector2::operator*(const float& value) const
{
	return Vector2(x * value, y * value);
}

Vector2 Vector2::operator+(const Vector2& value) const
{
	return Vector2(x + value.x, y + value.y);
}

Vector2 Vector2::operator-(const Vector2& value) const
{
	return Vector2(x - value.x, y - value.y);
}

Vector2 Vector2::operator-(void) const
{
	return Vector2(-x, -y);
}

const mfbs::Vector2* Vector2::Serialize(flatbuffers::FlatBufferBuilder& fbb) const
{
	MORTY_UNUSED(fbb);
	
	return reinterpret_cast<const mfbs::Vector2*>(this);
}

void Vector2::Deserialize(const void* pBufferPointer)
{
	memcpy(this, pBufferPointer, sizeof(Vector2));
}

const mfbs::Vector3* Vector3::Serialize(flatbuffers::FlatBufferBuilder& fbb) const
{
	MORTY_UNUSED(fbb);

	return reinterpret_cast<const mfbs::Vector3*>(this);
}

void Vector3::Deserialize(const void* pBufferPointer)
{
	memcpy(this, pBufferPointer, sizeof(Vector3));
}

const mfbs::Vector4* Vector4::Serialize(flatbuffers::FlatBufferBuilder& fbb) const
{
	MORTY_UNUSED(fbb);

	return reinterpret_cast<const mfbs::Vector4*>(this);
}

void Vector4::Deserialize(const void* pBufferPointer)
{
	memcpy(this, pBufferPointer, sizeof(Vector4));
}

Vector2i::Vector2i(int x, int y)
    :x(x)
    ,y(y)
{
}

bool Vector2i::operator!=(const Vector2i& other) const
{
	return !operator==(other);
}

bool Vector2i::operator==(const Vector2i& other) const
{
	return x == other.x && y == other.y;
}

Vector3i::Vector3i(int x, int y, int z)
	: x(x)
	, y(y)
	, z(z)
{
}

Vector3i Vector3i::operator*(const int& value) const
{
	return Vector3i(x * value, y * value, z * value);
}
