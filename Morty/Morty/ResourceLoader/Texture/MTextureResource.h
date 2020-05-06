/**
 * @File         MTextureResource
 * 
 * @Created      2019-08-29 16:35:04
 *
 * @Author       Pobrecito
**/

#ifndef _M_MTEXTURERESOURCE_H_
#define _M_MTEXTURERESOURCE_H_
#include "MGlobal.h"
#include "MResource.h"

#include "MTexture.h"

class MORTY_CLASS MTextureResource : public MResource
{
public:
	M_RESOURCE(MTextureResource)
    MTextureResource();
    virtual ~MTextureResource();

	MTexture* GetTextureTemplate(){ return m_pTexture; }
	
public:

	virtual void OnDelete() override;

protected:

	virtual bool Load(const MString& strResourcePath) override;
private:


	MTexture* m_pTexture;
};


#endif
