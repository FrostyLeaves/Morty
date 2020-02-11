/**
 * @File         MIMeshInstance
 * 
 * @Created      2019-12-13 20:24:19
 *
 * @Author       Pobrecito
**/

#ifndef _M_MIMESHINSTANCE_H_
#define _M_MIMESHINSTANCE_H_
#include "MGlobal.h"
#include "M3DNode.h"

class MIMesh;
class MMaterial;
class MBoundsOBB;
class MBoundsAABB;
class MModelInstance;
class MORTY_CLASS MIMeshInstance : public M3DNode
{
public:
	enum MEShadowType
	{
		ENone = 0,
		EOnlyDirectional = 1,
		EAllLights = 2,
	};

public:
	M_OBJECT(MIMeshInstance);
    MIMeshInstance();
    virtual ~MIMeshInstance();

public:
	virtual void SetMaterial(MMaterial* pMaterial) = 0;
	virtual MMaterial* GetMaterial() = 0;

	virtual MBoundsAABB* GetBoundsAABB() = 0;

public:
	virtual void SetMesh(MIMesh* pMesh) = 0;
	virtual MIMesh* GetMesh() = 0;

	void SetAttachedModelInstance(MModelInstance* pModelIns) { m_pModelInstance = pModelIns; }
	MModelInstance* GetAttachedModelInstance() { return m_pModelInstance; }

private:
	MModelInstance* m_pModelInstance;
};

#endif
