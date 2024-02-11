#pragma once

#include "Utility/MGlobal.h"
#include "Type/MType.h"

MORTY_SPACE_BEGIN

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


	virtual void EngineTick(const float& fDelta) { MORTY_UNUSED (fDelta); }
	virtual void SceneTick(MScene* pScene, const float& fDelta) {  MORTY_UNUSED(pScene, fDelta); }


	void SetEngine(MEngine* pEngine);
	MEngine* GetEngine();

private:
	MEngine* m_pEngine;
};

MORTY_SPACE_END