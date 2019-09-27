/**
 * @File         MTransform
 * 
 * @Created      2019-09-26 17:58:30
 *
 * @Author       Morty
**/

#ifndef _M_MTRANSFORM_H_
#define _M_MTRANSFORM_H_
#include "MGlobal.h"
#include "Vector.h"
#include "Quaternion.h"

class MORTY_CLASS MTransform
{
public:
    MTransform();
    virtual ~MTransform();

public:

	void SetPosition(const Vector3& position) { m_v3Position = position; }
	Vector3 GetPosition() const { return m_v3Position; }

	void SetScale(const Vector3& scale) { m_v3Scale = scale; }
	Vector3 GetScale() const { return m_v3Scale; }

	void SetRotation(const Quaternion& rotation){ m_qtRotation = rotation; }
	Quaternion GetRotation() const { return m_qtRotation; }

	Matrix4 GetMatrix();

	Vector3 GetUp() { return Vector3(0, 1, 0) * m_qtRotation.GetMatrix(); }
	Vector3 GetFront() { return Vector3(0, 0, 1) * m_qtRotation.GetMatrix(); }
	Vector3 GetRight() { return Vector3(1, 0, 0) * m_qtRotation.GetMatrix(); }

private:

	Vector3 m_v3Position;
	Vector3 m_v3Scale;
	Quaternion m_qtRotation;

};


#endif
