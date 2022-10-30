#ifndef _M_SUB_SYSTEM_H_
#define _M_SUB_SYSTEM_H_

#include "Utility/MGlobal.h"
#include "Type/MType.h"

class MScene;
class MEngine;
class MORTY_API MISubSystem : public MTypeClass
{
public:
	MORTY_INTERFACE(MISubSystem)

public:

	MISubSystem();
	virtual ~MISubSystem();

	virtual void Initialize() {};
	virtual void Release() {};


	virtual void SceneTick(MScene* pScene, const float& fDelta) {}


	void SetScene(MScene* pScene);
	MScene* GetScene();
	MEngine* GetEngine();

private:
	MScene* m_pScene;
};

#endif