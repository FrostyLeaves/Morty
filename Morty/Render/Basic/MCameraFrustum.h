/**
 * @File         MCameraFrustum
 * 
 * @Created      2020-04-14 22:28:43
 *
 * @Author       DoubleYe
 *
 * https://old.cescg.org/CESCG-2002/DSykoraJJelinek/
**/

#ifndef _M_MCAMERAFRUSTUM_H_
#define _M_MCAMERAFRUSTUM_H_
#include "MGlobal.h"
#include "MBounds.h"
#include "MPlane.h"

class MViewport;
class MORTY_API MCameraFrustum
{
public:
	enum MEContainType
	{
		EOUTSIDE = 0,	//在外部
		ENOTOUTSIDE = 1,//不在外部
		EINSIDE = 3,	//包含
		EINTERSECT = 5,	//交叉
	};


	MCameraFrustum();
	virtual ~MCameraFrustum();

public:

	void UpdateFromCameraInvProj(const Matrix4& m4CameraInvProj);

	MEContainType ContainTest(const Vector3& position);
	MEContainType ContainTest(const MBoundsAABB& aabb);
	MEContainType ContainTest(const MBoundsSphere& sphere);

	//只对某个方向指向的视锥体截面做测试
	MEContainType ContainTest(const MBoundsAABB& aabb, const Vector3& v3Direction);

	std::vector<MCameraFrustum> CutFrustum(const std::vector<float>& percent);

protected:

	MPlane m_vPlanes[6];
};


#endif
