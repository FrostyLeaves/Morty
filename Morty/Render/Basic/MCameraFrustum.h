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
#include "Utility/MGlobal.h"
#include "Utility/MBounds.h"
#include "Basic/MPlane.h"

class MViewport;
class MORTY_API MCameraFrustum
{
public:
	enum MEContainType
	{
		EOUTSIDE = 0,	//���ⲿ
		ENOTOUTSIDE = 1,//�����ⲿ
		EINSIDE = 3,	//����
		EINTERSECT = 5,	//����
	};


	MCameraFrustum();
	virtual ~MCameraFrustum();

public:

	const MPlane& GetPlane(const size_t& idx) const;

	void UpdateFromCameraInvProj(const Matrix4& m4CameraInvProj);

	MEContainType ContainTest(const Vector3& position);
	MEContainType ContainTest(const MBoundsAABB& aabb);
	MEContainType ContainTest(const MBoundsSphere& sphere);


	MEContainType ContainTest(const MBoundsAABB& aabb, const Vector3& v3Direction);

	std::vector<MCameraFrustum> CutFrustum(const std::vector<float>& percent);

protected:

	MPlane m_vPlanes[6];
};


#endif
