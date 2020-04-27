#ifndef _PROPERTY_VIEW_H_
#define _PROPERTY_VIEW_H_

#include "IBaseView.h"

#include "Vector.h"
#include "MString.h"
#include "MTransform.h"
#include "MVariant.h"

#include <map>
#include <deque>
#include <functional>

class MObject;
class M3DNode;
class PropertyBase;
class PropertyView : public IBaseView
{
public:
	PropertyView();
	virtual ~PropertyView();


public:
	void SetEditorObject(MObject* pObject);

	virtual void Render() override;

	virtual void Initialize(MEngine* pEngine) override;
	virtual void Release() override;

	virtual void Input(MInputEvent* pEvent) override;


protected:

	void EditObject(MObject* pObject);
	void CreatePropertyList(MObject* pObject);

private:
	MObject* m_pEditorObject;
	std::deque<PropertyBase*> m_vPropertyList;
	
	std::map<MString, std::function<PropertyBase*()>> m_tCreatePropertyFactory;
};


#endif