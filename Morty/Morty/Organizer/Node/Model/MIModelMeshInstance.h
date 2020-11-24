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

	void SetDetailLevel(const uint32_t& unLevel) { m_unDetailLevel = unLevel; }
	uint32_t GetDetailLevel() { return m_unDetailLevel; }

	void SetAttachedModelInstance(MModelInstance* pModelIns) { m_pModelInstance = pModelIns; }
	MModelInstance* GetAttachedModelInstance();

	void SetDrawBoundingSphere(const bool& bDrawable) { m_bDrawBoundingSphere = bDrawable; }
	bool GetDrawBoundingSphere() { return m_bDrawBoundingSphere; }

	void SetGenerateDirLightShadow(const bool& bGenerate) { m_bGenerateDirLightShadow = bGenerate; }
	bool GetGenerateDirLightShadow() const;

public:
	virtual void WriteToStruct(MStruct& srt) override;
	virtual void ReadFromStruct(MStruct& srt) override;


protected:
	virtual void ParentChangeImpl(MNode* pParent) override;

private:
	MEShadowType m_eShadowType;
	uint32_t m_unDetailLevel;

	bool m_bModelInstanceFound;
	MModelInstance* m_pModelInstance;

	bool m_bDrawBoundingSphere;
	bool m_bGenerateDirLightShadow;
};

#endif
