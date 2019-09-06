/**
 * @File         M3DNode
 * 
 * @Created      2019-05-25 19:54:30
 *
 * @Author       Morty
**/

#ifndef _M_M3DNODE_H_
#define _M_M3DNODE_H_
#include "MGlobal.h"

#include "MNode.h"

#include "Quaternion.h"
#include "Matrix.h"

class MORTY_CLASS M3DNode : public MNode
{
public:
    M3DNode();
    virtual ~M3DNode();


	void SetPosition(const Vector3& pos);
	Vector3 GetPosition();

	void SetRotation(const Quaternion& quat);
	Quaternion GetRotation();

	Matrix4 GetWorldTransform();
	Matrix4 GetRelativeTransform();

	void UpdateWorldTransform();

protected:
	void WorldTransformDirty();

private:

	Matrix4 m_m4Transform;
	bool m_bWorldTransformDirty;
	Matrix4 m_m4WorldTransform;
};


#endif
