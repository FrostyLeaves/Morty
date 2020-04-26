#ifndef _MATERIAL_VIEW_H_
#define _MATERIAL_VIEW_H_

#include "MGlobal.h"
#include "MResource.h"
#include "Property/PropertyBase.h"

class MScene;
class MEngine;
class MMaterial;
class MIMeshInstance;
class MaterialView
{
public:
	MaterialView();
	virtual ~MaterialView();


public:
	void SetMaterial(MMaterial* pMaterial);

	void UpdateMaterialTexture();

	void Render();

	void Initialize(MEngine* pEngine);
	void Release();

protected:

private:
	MResourceHolder* m_pResource;
	MMaterial* m_pMaterial;
	PropertyBase m_propertyBase;

	MScene* m_pScene;
	MEngine* m_pEngine;
	MIMeshInstance* m_pMeshInstance;

	class MTextureRenderTarget* m_pTextureRenderTarget;
	class MViewport* m_pRenderViewport;
};







#endif