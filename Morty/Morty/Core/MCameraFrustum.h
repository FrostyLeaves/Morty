/**
 * @File         MCameraFrustum
 * 
 * @Created      2020-04-14 22:28:43
 *
 * @Author       Pobrecito
**/

#ifndef _M_MCAMERAFRUSTUM_H_
#define _M_MCAMERAFRUSTUM_H_
#include "MGlobal.h"
#include "MBounds.h"
#include "MPlane.h"

class MViewport;
class MORTY_CLASS MCameraFrustum
{
public:
	enum MEContainType
	{
		EOUTSIDE = 0,	//在外部
		EINSIDE = 1,	//包含
		EINTERSECT = 2,	//交叉
	};


	MCameraFrustum();
	virtual ~MCameraFrustum();

public:

	void UpdateFromViewport(const MViewport& viewport);

protected:

	MEContainType ContainTest(const Vector3& position);
	MEContainType ContainTest(const MBoundsAABB& aabb);
	MEContainType ContainTest(const MBoundsSphere& sphere);

	MPlane m_vPlanes[6];

	unsigned int m_vNearIndex[6];
	unsigned int m_vFarIndex[6];
};


#endif
