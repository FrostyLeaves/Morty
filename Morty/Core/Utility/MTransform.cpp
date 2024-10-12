#include "Utility/MTransform.h"
#include "Flatbuffer/MTransform_generated.h"

using namespace morty;

MTransform::MTransform()
    : m_position(0, 0, 0)
    , m_scale(1, 1, 1)
    , m_qtRotation()
{}

MTransform::MTransform(const Matrix4& matTransform)
{
    m_position   = matTransform.GetTranslation();
    m_scale      = matTransform.GetScale();
    m_qtRotation = matTransform.GetRotation();
    m_qtRotation.Normalize();
}

MTransform::~MTransform() {}

Matrix4 MTransform::GetMatrix() const
{
    Matrix4 matmove = Matrix4::IdentityMatrix;
    matmove.m[0][3] = m_position.x;
    matmove.m[1][3] = m_position.y;
    matmove.m[2][3] = m_position.z;

    Matrix4 matrota = m_qtRotation;

    Matrix4 matscal = Matrix4::IdentityMatrix;
    matscal.m[0][0] = m_scale.x;
    matscal.m[1][1] = m_scale.y;
    matscal.m[2][2] = m_scale.z;

    return matmove * matrota * matscal;
}

flatbuffers::Offset<void> MTransform::Serialize(flatbuffers::FlatBufferBuilder& fbb)
{
    fbs::MTransformBuilder fbBuilder(fbb);

    fbBuilder.add_position(m_position.Serialize(fbb));
    fbBuilder.add_scale(m_scale.Serialize(fbb));
    fbBuilder.add_rotation(m_qtRotation.Serialize(fbb));

    return fbBuilder.Finish().Union();
}

void MTransform::Deserialize(const void* pBufferPointer)
{
    const fbs::MTransform* fbTransform =
            reinterpret_cast<const fbs::MTransform*>(pBufferPointer);

    m_position.Deserialize(fbTransform->position());
    m_qtRotation.Deserialize(fbTransform->rotation());
    m_scale.Deserialize(fbTransform->scale());
}

TEST_CASE("core transform test")
{
    MTransform transform;
    transform.SetPosition(Vector3(10, 0, 0));
    transform.SetRotation(Quaternion(Vector3(0, 1, 0), 45.0f));
    transform.SetScale(Vector3(1, 2, 3));

    MTransform other = transform.GetMatrix();

    Vector3    v3TestPosition = other.GetPosition() - transform.GetPosition();
    CHECK(v3TestPosition.Length() < MGlobal::M_FLOAT_BIAS);

    Vector3 v3TestQuaternion =
            other.GetRotation().GetEulerAngle() - transform.GetRotation().GetEulerAngle();
    CHECK(v3TestQuaternion.Length() < MGlobal::M_FLOAT_BIAS);

    Vector3 v3TestScale = other.GetScale() - transform.GetScale();
    CHECK(v3TestScale.Length() < MGlobal::M_FLOAT_BIAS);
}