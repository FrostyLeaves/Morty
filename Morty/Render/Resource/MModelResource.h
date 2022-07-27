/**
 * @File         MModelResource
 * 
 * @Created      2020-06-14 16:58:57
 *
 * @Author       DoubleYe
**/

#ifndef _M_MMODELRESOURCE_H_
#define _M_MMODELRESOURCE_H_
#include "MGlobal.h"
#include "MResource.h"

#include "MMeshResource.h"
#include "MSkeletonResource.h"

class MSkeleton;
class MSkeletalAnimation;
class MORTY_API MModelResource : public MResource
{
public:
	MORTY_INTERFACE(MModelResource);
    MModelResource();
    virtual ~MModelResource();

public:

	static MString GetResourceTypeName() { return "Model"; }
	static std::vector<MString> GetSuffixList() { return { "model" }; }

    std::shared_ptr<MSkeletonResource> GetSkeleton() { return m_pSkeleton; }
    const std::vector<std::shared_ptr<MMeshResource>>& GetMeshResources() { return m_vMeshes; }

    void SetSkeletonResource(std::shared_ptr<MSkeletonResource> pSkeleton);
    void GetMeshResources(const std::vector<std::shared_ptr<MMeshResource>>& vMeshes);

public:

    virtual bool Load(const MString& strResourcePath) override;

    virtual bool SaveTo(const MString& strResourcePath) override;

    virtual void OnDelete() override;

private:

    friend class MModelConverter;

    std::shared_ptr<MSkeletonResource> m_pSkeleton;
    std::vector<std::shared_ptr<MMeshResource>> m_vMeshes;
};

#endif
