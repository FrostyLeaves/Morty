/**
 * @File         MTransform
 * 
 * @Created      2019-09-26 17:58:30
 *
 * @Author       DoubleYe
**/

#ifndef _M_MTRANSFORM_H_
#define _M_MTRANSFORM_H_
#include "Utility/MGlobal.h"
#include "Math/Vector.h"
#include "Math/Matrix.h"
#include "Math/Quaternion.h"

class MORTY_API MTransform
{
public:
    MTransform();
	MTransform(const Matrix4& matTransform);
    virtual ~MTransform();

public:

	void SetPosition(const Vector3& position) { m_v3Position = position; }
	Vector3 GetPosition() const { return m_v3Position; }

	void SetScale(const Vector3& scale) { m_v3Scale = scale; }
	Vector3 GetScale() const { return m_v3Scale; }

	void SetRotation(const Quaternion& rotation){ m_qtRotation = rotation; }
	Quaternion GetRotation() const { return m_qtRotation; }

	Matrix4 GetMatrix() const;

	Matrix4 GetMatrixScale();

	Vector3 GetUp() { return Matrix4(m_qtRotation) * Vector3(0, 1, 0); }
	Vector3 GetForward() { return Matrix4(m_qtRotation) * Vector3(0, 0, 1); }
	Vector3 GetRight() { return Matrix4(m_qtRotation) * Vector3(1, 0, 0); }

private:

	Vector3 m_v3Position;
	Vector3 m_v3Scale;
	Quaternion m_qtRotation;

};


#endif
