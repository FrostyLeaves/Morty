/**
 * @File         MModelResource
 * 
 * @Created      2020-06-14 16:58:57
 *
 * @Author       Pobrecito
**/

#ifndef _M_MMODELRESOURCE_H_
#define _M_MMODELRESOURCE_H_
#include "MGlobal.h"
#include "MResource.h"

#include "MMeshResource.h"
#include "MSkeletonResource.h"

class MORTY_CLASS MModelResource : public MResource
{
public:
	M_RESOURCE(MModelResource);
    MModelResource();
    virtual ~MModelResource();

public:

    virtual bool Load(const MString& strResourcePath) override;

    virtual bool SaveTo(const MString& strResourcePath) override;

private:

    MSkeletonResource m_Skeleton;
    std::vector<MMeshResource> m_Meshes;
};

#endif
