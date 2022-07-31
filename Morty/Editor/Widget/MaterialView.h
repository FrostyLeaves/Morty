#ifndef _MATERIAL_VIEW_H_
#define _MATERIAL_VIEW_H_

#include "Main/IBaseView.h"
#include "Render/SceneTexture.h"

#include "Resource/MResource.h"
#include "Property/PropertyBase.h"

class MScene;
class MEntity;
class MEngine;
class MMaterial;
class MInputEvent;
class MStaticMeshInstance;
class MaterialView : public IBaseView
{
public:
	MaterialView();
	virtual ~MaterialView();

public:
	void SetMaterial(std::shared_ptr<MMaterial> pMaterial);

	SceneTexture& GetSceneTexture() { return m_SceneTexture; }

	virtual void Render() override;

	virtual void Initialize(MEngine* pEngine) override;
	virtual void Release() override;

	virtual void Input(MInputEvent* pEvent) override;

protected:

private:
	MResourceKeeper m_Resource;
	std::shared_ptr<MMaterial> m_pMaterial;
	PropertyBase m_propertyBase;

	MEngine* m_pEngine;

	MEntity* m_pStaticSphereMeshNode;
	MEntity* m_pSkeletonSphereMeshNode;

	bool m_bShowPreview;
	SceneTexture m_SceneTexture;
};

#endif