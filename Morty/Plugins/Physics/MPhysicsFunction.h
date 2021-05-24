/**
 * @File         MPhysicsFunction
 * 
 * @Created      2021-05-20 14:51:20
 *
 * @Author       Pobrecito
**/

#ifndef _M_MPHYSICSFUNCTION_H_
#define _M_MPHYSICSFUNCTION_H_
#include "MGlobal.h"

#include "MTransform.h"

class btVector3;
class btMatrix3x3;
class btTransform;
class btQuaternion;
class MORTY_API MPhysicsFunction
{
private:
    MPhysicsFunction();
    virtual ~MPhysicsFunction();

public:

    static void ConvertTransform(const MTransform& transform0, btTransform* transform1);
    static void ConvertTransform(const btTransform* transform0, MTransform& transform1);
    static void ConvertTransform(const Matrix4& transform0, btTransform* transform1);

    static void ConvertVector3(const Vector3& vector0, btVector3* vector1);
    static void ConvertVector3(const btVector3* vector0, Vector3& vector1);

    static void ConvertMatrix3(const Matrix3& matrix0, btMatrix3x3* matrix1);
    static void ConvertMatrix3(const btMatrix3x3* matrix0, Matrix3& matrix1);

    static void ConvertQuaternion(const btQuaternion* quat0, Quaternion& quat1);

private:

};


#endif
