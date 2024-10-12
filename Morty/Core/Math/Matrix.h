#pragma once

#include "Utility/MGlobal.h"

#include "Math/Quaternion.h"
#include "Math/Vector.h"

namespace morty
{

namespace fbs
{
struct Matrix3;
struct Matrix4;
}// namespace fbs

class Vector3;
class Vector4;
class Quaternion;
class Matrix3;
class Matrix4;
class Matrix2
{
public:
    Matrix2();
    Matrix2(const Matrix3& mat3, const int& i, const int& j);

public:
    float m[2][2];
};

class MORTY_API Matrix3
{
public:
    Matrix3();
    Matrix3(const Matrix4& mat4, const int& i, const int& j);
    Matrix3(const float* vValues);
    Matrix3(const float& m_00,
            const float& m_01,
            const float& m_02,
            const float& m_10,
            const float& m_11,
            const float& m_12,
            const float& m_20,
            const float& m_21,
            const float& m_22);

    Matrix3 Transposed() const;

    float   Determinant() const;

    Matrix3 CofactorMatrix() const;
    Matrix3 AdjointMatrix() const;
    Matrix3 Inverse() const;


    Matrix3 operator*(const float& value) const;
    Matrix3 operator/(const float& value) const;

    Matrix3 operator*(const Matrix3& mat) const;
    Vector3 operator*(const Vector3& value) const;

public:
    const fbs::Matrix3* Serialize(flatbuffers::FlatBufferBuilder& fbb) const;
    void                Deserialize(const void* pBufferPointer);

public:
    float m[3][4];

private:
    float AlgebraicCofactor(const int& i, const int& j) const;
};

class MORTY_API Matrix4
{
public:
    Matrix4();
    Matrix4(float value[4][4]);
    Matrix4(float value[16]);
    Matrix4(const float& m_00,
            const float& m_01,
            const float& m_02,
            const float& m_03,
            const float& m_10,
            const float& m_11,
            const float& m_12,
            const float& m_13,
            const float& m_20,
            const float& m_21,
            const float& m_22,
            const float& m_23,
            const float& m_30,
            const float& m_31,
            const float& m_32,
            const float& m_33);

    Matrix4(const Quaternion& quat);

    Matrix4    Transposed() const;

    float      Determinant() const;

    Matrix4    CofactorMatrix() const;
    Matrix4    AdjointMatrix() const;
    Matrix4    Inverse() const;

    bool       IsOrthogonal() const;

    Matrix4    operator*(const Matrix4& mat) const;
    Matrix4    operator*(const float& value) const;
    Matrix4    operator/(const float& value) const;

    Vector4    operator*(const Vector4& value) const;
    Vector3    operator*(const Vector3& value) const;

    Vector4    Row(const unsigned int& row) const;

    Vector4    Col(const unsigned int& col) const;


    bool       operator==(const Matrix4& mat) const;

    Vector3    GetTranslation() const;
    //
    Quaternion GetRotation() const;
    //
    Matrix4    GetRotatePart() const;
    Matrix4    GetScalePart() const;

    Vector3    GetScale() const;


public:
    const fbs::Matrix4* Serialize(flatbuffers::FlatBufferBuilder& fbb) const;
    void                Deserialize(const void* pBufferPointer);

public:
    float                m[4][4];

    static const Matrix4 IdentityMatrix;

private:
    float AlgebraicCofactor(const int& i, const int& j) const;
};

}// namespace morty