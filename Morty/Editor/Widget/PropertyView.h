#ifndef _PROPERTY_VIEW_H_
#define _PROPERTY_VIEW_H_

#include "IBaseView.h"

#include "Vector.h"
#include "MColor.h"
#include "MString.h"
#include "MTransform.h"
#include "MVariant.h"

#include <map>
#include <deque>
#include <functional>

class MEntity;
class PropertyBase;
class PropertyView : public IBaseView
{
public:
	PropertyView();
	virtual ~PropertyView();


public:
	void SetEditorNode(MEntity* pNode);

	virtual void Render() override;

	virtual void Initialize(MEngine* pEngine) override;
	virtual void Release() override;

	virtual void Input(MInputEvent* pEvent) override;


protected:

	void EditEntity(MEntity* pNode);
	void CreatePropertyList(MEntity* pNode);

private:
	MEntity* m_pEditorNode;
	std::deque<PropertyBase*> m_vPropertyList;
	
	std::map<MString, std::function<PropertyBase*()>> m_tCreatePropertyFactory;
};


#endif