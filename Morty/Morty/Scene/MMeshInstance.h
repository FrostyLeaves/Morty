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

class MMaterial;
class MResource;
class MModelResource;
class MORTY_CLASS MMeshInstance : public M3DNode
{
public:
    MMeshInstance();
    virtual ~MMeshInstance();

public:

	bool Load(MResource* pResource);

	void Test_SetMaterial(MMaterial* pMaterial);
	MMaterial* Test_GetMaterial(){ return m_pMaterial; }

	virtual void OnTick(const float& fDelta);
	virtual void Render();

	MModelResource* GetResource(){ return m_pResource; }

private:

	MModelResource* m_pResource;


	MMaterial* m_pMaterial;
};


#endif
