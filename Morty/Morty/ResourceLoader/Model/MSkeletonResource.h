/**
 * @File         MSkeletonResource
 * 
 * @Created      2020-06-02 01:50:44
 *
 * @Author       Pobrecito
**/

#ifndef _M_MSKELETONRESOURCE_H_
#define _M_MSKELETONRESOURCE_H_
#include "MGlobal.h"
#include "MSerializer.h"
#include "MResource.h"
#include "MSkeleton.h"

class MORTY_CLASS MSkeletonResource : public MResource, public MSerializer
{
public:
	M_RESOURCE(MSkeletonResource);
    MSkeletonResource();
    virtual ~MSkeletonResource();

public:

    void SetSkeleton(MSkeleton* pSkeleton);
    MSkeleton* GetSkeleton() { return m_pSkeletonTemplate; }


protected:

    void Clean();


	virtual void WriteToStruct(MStruct& srt) override;
	virtual void ReadFromStruct(MStruct& srt) override;

    virtual bool Load(const MString& strResourcePath) override;
    virtual bool SaveTo(const MString& strResourcePath) override;

private:

    MSkeleton* m_pSkeletonTemplate;

};

#endif
