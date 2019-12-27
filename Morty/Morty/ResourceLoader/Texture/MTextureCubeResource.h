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
class MResourceHolder;
class MORTY_CLASS MTextureCubeResource : public MResource
{
public:
	MTextureCubeResource();
	virtual ~MTextureCubeResource();

	void SetTextures(MTextureResource* vTexs[6]);

	MTextureCube* GetTextureCubeTemplate(){ return m_pTextureCube; }

protected:

	virtual bool Load(const MString& strResourcePath) override;
private:


	MTextureCube* m_pTextureCube;

	MResourceHolder* m_vTextures[6];
};


#endif
