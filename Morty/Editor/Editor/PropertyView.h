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

class MNode;
class PropertyBase;
class PropertyView : public IBaseView
{
public:
	PropertyView();
	virtual ~PropertyView();


public:
	void SetEditorNode(MNode* pNode);

	virtual void Render() override;

	virtual void Initialize(MEngine* pEngine) override;
	virtual void Release() override;

	virtual void Input(MInputEvent* pEvent) override;


protected:

	void EditNode(MNode* pNode);
	void CreatePropertyList(MNode* pNode);

private:
	MNode* m_pEditorNode;
	std::deque<PropertyBase*> m_vPropertyList;
	
	std::map<MString, std::function<PropertyBase*()>> m_tCreatePropertyFactory;
};


#endif