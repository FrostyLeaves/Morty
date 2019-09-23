/**
 * @File         MSkyBox
 * 
 * @Created      2019-09-22 01:39:38
 *
 * @Author       Morty
**/

#ifndef _M_MSKYBOX_H_
#define _M_MSKYBOX_H_
#include "MGlobal.h"
#include "MObject.h"

class MResource;
class MResourceHolder;
class MTextureCubeResource;
class MTextureCube;
class MMeshInstance;
class MORTY_CLASS MSkyBox : public MObject
{
public:
    MSkyBox();
    virtual ~MSkyBox();

	MTextureCube* GetTextureCube(){ return m_pTextureCube; }

	virtual bool Load(MResource* pResource);

	virtual void OnCreated() override;

	MMeshInstance* GetMeshInstance(){ return m_pMeshInstance; }

public:

private:

	MMeshInstance* m_pMeshInstance;

	MTextureCube* m_pTextureCube;
	MResourceHolder* m_pTextureCubeResource;

};


#endif
