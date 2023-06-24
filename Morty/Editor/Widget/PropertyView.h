#ifndef _PROPERTY_VIEW_H_
#define _PROPERTY_VIEW_H_

#include "Main/IBaseView.h"

#include "Math/Vector.h"
#include "Utility/MColor.h"
#include "Utility/MString.h"
#include "Utility/MTransform.h"

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
	
	virtual void Render() override;

	virtual void Initialize(MEngine* pEngine) override;
	virtual void Release() override;

	virtual void Input(MInputEvent* pEvent) override;


protected:

	void EditEntity(MEntity* pNode);
	void CreatePropertyList(MEntity* pNode);

private:
	std::deque<PropertyBase*> m_vPropertyList;
	
	std::map<MString, std::function<PropertyBase*()>> m_tCreatePropertyFactory;
};


#endif