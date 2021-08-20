#ifndef _M_SYSTEM_H_
#define _M_SYSTEM_H_

#include "MGlobal.h"
#include "MType.h"

class MScene;
class MEngine;
class MORTY_API MISystem : public MTypeClass
{
public:
	MORTY_INTERFACE(MISystem)

public:

	MISystem();
	virtual ~MISystem();

	virtual void Initialize() {};
	virtual void Release() {};


	virtual void EngineTick(const float& fDelta) {}
	virtual void SceneTick(MScene* pScene, const float& fDelta) {}


	void SetEngine(MEngine* pEngine);
	MEngine* GetEngine();

private:
	MEngine* m_pEngine;
};

#endif