#pragma once

#include "Utility/MGlobal.h"

#include "Math/Matrix.h"
#include "Math/Vector.h"

namespace morty
{

namespace fbs
{
struct Quaternion;
}

class Vector3;
class Matrix4;
class MORTY_API Quaternion
{
public:
    Quaternion();

    Quaternion(const float& w, const float& x, const float& y, const float& z);

    Quaternion(const float& w, const Vector3& v);

    Quaternion(const Vector3& vAxis, const float& fAngle);

    Quaternion(const fbs::Quaternion& value);

    static Quaternion FromEuler(const Vector3& eulerVec3);

public:
    const fbs::Quaternion* Serialize(flatbuffers::FlatBufferBuilder& fbb) const;

    void                   Deserialize(const void* pBufferPointer);

public:
    float w;
    union
    {
        struct {
            float x;
            float y;
            float z;
        };
        float m[3];
    };


public:
    Quaternion        operator-(void) const;

    Quaternion        operator*(const Quaternion& quat) const;

    Quaternion        CrossProduct(const Quaternion& quat) const;

    float             DotProduct(const Quaternion& quat) const;

    float             Length() const;

    void              Normalize();

    void              RotateX(const float& fAngle);

    void              RotateY(const float& fAngle);

    void              RotateZ(const float& fAngle);

    void              RotateAxis(Vector3 axis, const float& fAngle);

    float             GetAngle();

    Vector3           GetAxis();

    Vector3           GetEulerAngle() const;

    void              SetEulerAngle(const Vector3& eulerVec3);

    static Quaternion Slerp(const Quaternion& quat1, Quaternion quat2, const float fSmooth);
};


const Quaternion UnitQuaternion(1, 0, 0, 0);

}// namespace morty