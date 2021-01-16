/**
 * @File         MRenderPassResource
 * 
 * @Created      2020-12-25 21:08:21
 *
 * @Author       Pobrecito
**/

#ifndef _M_MRENDERPASSRESOURCE_H_
#define _M_MRENDERPASSRESOURCE_H_
#include "MGlobal.h"
#include "MResource.h"

class MRenderPass;
class MORTY_API MRenderPassResource : public MResource
{
public:
	M_RESOURCE(MRenderPassResource);

    MRenderPassResource();
    virtual ~MRenderPassResource();

public:

	MRenderPass* GetRenderPassTemplate() { return m_pRenderPass; }

protected:

	virtual bool Load(const MString& strResourcePath) override;
    virtual bool SaveTo(const MString& strResourcePath) override;

private:


	MRenderPass* m_pRenderPass;
};


#endif
