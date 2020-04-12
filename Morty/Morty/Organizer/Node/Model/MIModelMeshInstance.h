/**
 * @File         MIModelMeshInstance
 * 
 * @Created      2020-03-21 19:46:47
 *
 * @Author       Pobrecito
**/

#ifndef _M_MIMODELMESHINSTANCE_H_
#define _M_MIMODELMESHINSTANCE_H_
#include "MGlobal.h"
#include "MIMeshInstance.h"

class MORTY_CLASS MIModelMeshInstance : public MIMeshInstance
{
public:
	enum MEShadowType
	{
		ENone = 0,
		EOnlyDirectional = 1,
		EAllLights = 2,
	};

public:
	M_OBJECT(MIModelMeshInstance);
	MIModelMeshInstance();
    virtual ~MIModelMeshInstance();

public:
	void SetShadowType(const MEShadowType& eType) { m_eShadowType = eType; }
	MEShadowType GetShadowType() { return m_eShadowType; }

	void SetDetailLevel(const unsigned int& unLevel) { m_unDetailLevel = unLevel; }
	unsigned int GetDetailLevel() { return m_unDetailLevel; }

	void SetAttachedModelInstance(MModelInstance* pModelIns) { m_pModelInstance = pModelIns; }
	MModelInstance* GetAttachedModelInstance() { return m_pModelInstance; }

	void SetDrawBoundingSphere(const bool& bDrawable) { m_bDrawBoundingSphere = bDrawable; }
	bool GetDrawBoundingSphere() { return m_bDrawBoundingSphere; }

private:
	MEShadowType m_eShadowType;
	MModelInstance* m_pModelInstance;
	unsigned int m_unDetailLevel;

	bool m_bDrawBoundingSphere;
};

#endif
