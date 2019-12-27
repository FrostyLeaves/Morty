/**
 * @File         MStaticMeshInstance
 * 
 * @Created      2019-05-26 16:13:55
 *
 * @Author       Pobrecito
**/

#ifndef _M_MSTATICMESHINSTANCE_H_
#define _M_MSTATICMESHINSTANCE_H_
#include "MGlobal.h"

#include "MIMeshInstance.h"

class MIMesh;
class MMaterial;
class MResource;
class MModelResource;
class MORTY_CLASS MStaticMeshInstance : public MIMeshInstance
{
public:
	M_OBJECT(MStaticMeshInstance);
    MStaticMeshInstance();
    virtual ~MStaticMeshInstance();

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
