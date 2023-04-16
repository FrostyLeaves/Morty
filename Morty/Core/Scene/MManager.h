#ifndef _M_MANAGER_H_
#define _M_MANAGER_H_

#include "Utility/MGlobal.h"
#include "Type/MType.h"

class MScene;
class MEngine;
class MComponent;
class MORTY_API IManager : public MTypeClass
{
public:
	MORTY_INTERFACE(IManager)

public:

	IManager();
	virtual ~IManager();

	virtual void Initialize() {};
	virtual void Release() {};

	virtual std::set<const MType*> RegisterComponentType() const { return {}; }

	virtual void SceneTick(MScene* pScene, const float& fDelta) {}
	virtual void RegisterComponent(MComponent* pComponent) {}
	virtual void UnregisterComponent(MComponent* pComponent) {}

	void SetScene(MScene* pScene);
	MScene* GetScene();
	MEngine* GetEngine();

private:
	MScene* m_pScene;
};

#endif