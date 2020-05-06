/**
 * @File         MTextureCubeResource
 * 
 * @Created      2019-09-22 14:51:44
 *
 * @Author       Pobrecito
**/

#ifndef _M_MTEXTURECUBERESOURCE_H_
#define _M_MTEXTURECUBERESOURCE_H_
#include "MGlobal.h"
#include "MResource.h"

class MTextureCube;
class MTextureResource;
class MResourceKeeper;
class MORTY_CLASS MTextureCubeResource : public MResource
{
public:
	M_RESOURCE(MTextureCubeResource)
	MTextureCubeResource();
	virtual ~MTextureCubeResource();

	void SetTextures(MTextureResource* vTexs[6]);

	MTextureCube* GetTextureCubeTemplate(){ return m_pTextureCube; }

public:

	virtual void OnDelete() override;

protected:

	virtual bool Load(const MString& strResourcePath) override;
private:


	MTextureCube* m_pTextureCube;

	MResourceKeeper m_vTextures[6];
};


#endif
