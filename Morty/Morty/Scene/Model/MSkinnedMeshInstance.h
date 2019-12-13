/**
 * @File         MAnimationMeshInstance
 * 
 * @Created      2019-12-13 20:24:34
 *
 * @Author       Morty
**/

#ifndef _M_SKINNEDMESHINSTANCE_H_
#define _M_SKINNEDMESHINSTANCE_H_
#include "MGlobal.h"
#include "MIMeshInstance.h"

class MIMesh;
class MMaterial;
class MResource;
class MModelResource;
class MORTY_CLASS MSkinnedMeshInstance : public MIMeshInstance
{
public:
	M_OBJECT(MSkinnedMeshInstance);
    MSkinnedMeshInstance();
    virtual ~MSkinnedMeshInstance();

public:

	virtual void SetMesh(MIMesh* pMesh) override;
	virtual MIMesh* GetMesh() override { return m_pMesh; }

	virtual void SetMaterial(MMaterial* pMaterial) override;
	virtual MMaterial* GetMaterial() override { return m_pMaterial; }

private:

	MIMesh* m_pMesh;
	MMaterial* m_pMaterial;
};


#endif
