/**
 * @File         MMeshInstance
 * 
 * @Created      2019-05-26 16:13:55
 *
 * @Author       Morty
**/

#ifndef _M_MMESHINSTANCE_H_
#define _M_MMESHINSTANCE_H_
#include "MGlobal.h"

#include "M3DNode.h"

class MIMesh;
class MMaterial;
class MResource;
class MModelResource;
class MORTY_CLASS MMeshInstance : public M3DNode
{
public:
	M_OBJECT(MMeshInstance);
    MMeshInstance();
    virtual ~MMeshInstance();

public:

	void SetMesh(MIMesh* pMesh);
	MIMesh* GetMesh(){ return m_pMesh; }

	void SetMaterial(MMaterial* pMaterial);
	MMaterial* GetMaterial(){ return m_pMaterial; }

	virtual void OnTick(const float& fDelta);

private:

	MIMesh* m_pMesh;
	MMaterial* m_pMaterial;
};


#endif
