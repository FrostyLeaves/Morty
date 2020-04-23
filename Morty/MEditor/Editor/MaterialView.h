#ifndef _MATERIAL_VIEW_H_
#define _MATERIAL_VIEW_H_

#include "MGlobal.h"
#include "MResource.h"
#include "Property/PropertyBase.h"

class MMaterial;
class MaterialView
{
public:
	MaterialView();
	virtual ~MaterialView();


public:
	void SetMaterial(MMaterial* pMaterial);

	void Render();

protected:

private:
	MResourceHolder* m_pResource;
	MMaterial* m_pMaterial;
	PropertyBase m_propertyBase;
};







#endif