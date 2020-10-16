#ifndef _MATERIAL_VIEW_H_
#define _MATERIAL_VIEW_H_

#include "IBaseView.h"
#include "SceneTexture.h"

#include "MResource.h"
#include "Property/PropertyBase.h"

class MScene;
class MEngine;
class MMaterial;
class MInputEvent;
class MIMeshInstance;
class MaterialView : public IBaseView
{
public:
	MaterialView();
	virtual ~MaterialView();

public:
	void SetMaterial(MMaterial* pMaterial);

	void UpdateMaterialTexture();

	virtual void Render() override;

	virtual void Initialize(MEngine* pEngine) override;
	virtual void Release() override;

	virtual void Input(MInputEvent* pEvent) override;

protected:

private:
	MResourceKeeper m_Resource;
	MMaterial* m_pMaterial;
	PropertyBase m_propertyBase;

	MEngine* m_pEngine;
	MIMeshInstance* m_pMeshInstance;

	bool m_bShowPreview;
	SceneTexture m_SceneTexture;
};

#endif