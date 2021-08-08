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

    MSkeletonResource* GetSkeleton() { return m_pSkeleton; }
    const std::vector<MMeshResource*>& GetMeshResources() { return m_vMeshes; }

    void SetSkeletonResource(MSkeletonResource* pSkeleton);
    void GetMeshResources(const std::vector<MMeshResource*>& vMeshes);

public:

    virtual bool Load(const MString& strResourcePath) override;

    virtual bool SaveTo(const MString& strResourcePath) override;

    virtual void OnDelete() override;

private:

    friend class MModelConverter;

    MSkeletonResource* m_pSkeleton;
    std::vector<MMeshResource*> m_vMeshes;
};

#endif
