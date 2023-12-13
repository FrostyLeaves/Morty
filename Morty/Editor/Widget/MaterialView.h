#pragma once

#include "Main/BaseWidget.h"
#include "Render/SceneTexture.h"

#include "Resource/MResource.h"
#include "Property/PropertyBase.h"

class MScene;
class MEntity;
class MEngine;
class MMaterialResource;
class MInputEvent;
class MaterialView : public BaseWidget
{
public:
	MaterialView();
	~MaterialView() = default;

public:
	void SetMaterial(std::shared_ptr<MMaterialResource> pMaterial);

	void Initialize(MainEditor* pMainEditor) override;
	void Release() override;

	void Input(MInputEvent* pEvent) override;
	void Render() override;

	
private:
	std::shared_ptr<MMaterialResource> m_pMaterial = nullptr;
	PropertyBase m_propertyBase;
	
	MScene* m_pScene = nullptr;

	MEntity* m_pStaticSphereMeshNode = nullptr;
	MEntity* m_pSkeletonSphereMeshNode = nullptr;

	std::shared_ptr<SceneTexture> m_pSceneTexture = nullptr;
};
