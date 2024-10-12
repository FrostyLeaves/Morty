/**
 * @File         MCameraFrustum
 * 
 * @Created      2020-04-14 22:28:43
 *
 * @Author       DoubleYe
 *
 * https://old.cescg.org/CESCG-2002/DSykoraJJelinek/
**/

#pragma once

#include "Utility/MGlobal.h"
#include "Basic/MPlane.h"
#include "Utility/MBounds.h"

namespace morty
{

class MViewport;
class MORTY_API MCameraFrustum
{
public:
    enum MEContainType
    {
        EOUTSIDE    = 0,//���ⲿ
        ENOTOUTSIDE = 1,//�����ⲿ
        EINSIDE     = 3,//����
        EINTERSECT  = 5,//����
    };


    MCameraFrustum();

    virtual ~MCameraFrustum();

public:
    MPlane                      GetPlane(const size_t& idx) const;

    void                        UpdateFromCameraInvProj(const Matrix4& m4CameraInvProj);

    MEContainType               ContainTest(const Vector3& position) const;

    MEContainType               ContainTest(const MBoundsAABB& aabb) const;

    MEContainType               ContainTest(const MBoundsSphere& sphere) const;


    MEContainType               ContainTest(const MBoundsAABB& aabb, const Vector3& v3Direction) const;

    std::vector<MCameraFrustum> CutFrustum(const std::vector<float>& percent);

protected:
    MPlane m_planes[6];
};

}// namespace morty