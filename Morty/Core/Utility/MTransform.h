/**
 * @File         MTransform
 * 
 * @Created      2019-09-26 17:58:30
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"
#include "Math/Matrix.h"
#include "Math/Quaternion.h"
#include "Math/Vector.h"

namespace morty
{

class MORTY_API MTransform
{
public:
    MTransform();
    MTransform(const Matrix4& matTransform);
    virtual ~MTransform();

public:
    void       SetPosition(const Vector3& position) { m_position = position; }
    Vector3    GetPosition() const { return m_position; }

    void       SetScale(const Vector3& scale) { m_scale = scale; }
    Vector3    GetScale() const { return m_scale; }

    void       SetRotation(const Quaternion& rotation) { m_qtRotation = rotation; }
    Quaternion GetRotation() const { return m_qtRotation; }

    Matrix4    GetMatrix() const;

    Vector3    GetUp() const { return Matrix4(m_qtRotation) * Vector3(0, 1, 0); }
    Vector3    GetForward() const { return Matrix4(m_qtRotation) * Vector3(0, 0, 1); }
    Vector3    GetRight() const { return Matrix4(m_qtRotation) * Vector3(1, 0, 0); }

    flatbuffers::Offset<void> Serialize(flatbuffers::FlatBufferBuilder& fbb);
    void                      Deserialize(const void* pBufferPointer);

private:
    Vector3    m_position;
    Vector3    m_scale;
    Quaternion m_qtRotation;
};

}// namespace morty