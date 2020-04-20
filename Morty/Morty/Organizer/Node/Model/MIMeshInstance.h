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
#include "MBounds.h"

class MIMesh;
class MMaterial;
class MModelMeshStruct;
class MModelInstance;
class MORTY_CLASS MIMeshInstance : public M3DNode
{
public:
	enum MERenderOrderType
	{
		EAutoOrder = 0,
		EOrderByTransparent = 1,
		EOrderByMaterial = 2,
	};

public:
	M_OBJECT(MIMeshInstance);
    MIMeshInstance();
    virtual ~MIMeshInstance();

public:
	virtual void SetMaterial(MMaterial* pMaterial) = 0;
	virtual MMaterial* GetMaterial() = 0;

	virtual MBoundsAABB* GetBoundsAABB() = 0;
	virtual MBoundsSphere* GetBoundsSphere() = 0;

	virtual void SetGenerateDirLightShadow(const bool& bGenerate) = 0;
	virtual bool GetGenerateDirLightShadow() const = 0;


	MERenderOrderType GetRenderOrderType() { return m_eRenderOrderType; }
	void SetRenderOrderType(const MERenderOrderType& eType) { m_eRenderOrderType = eType; }

public:
	virtual MIMesh* GetMesh() = 0;
	virtual MIMesh* GetMesh(const unsigned int& unDetailLevel) = 0;

private:

	MERenderOrderType m_eRenderOrderType;
};

#endif
