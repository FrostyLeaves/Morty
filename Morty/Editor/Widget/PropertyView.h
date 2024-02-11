#pragma once

#include "Main/BaseWidget.h"

#include "Math/Vector.h"
#include "Utility/MColor.h"
#include "Utility/MString.h"
#include "Utility/MTransform.h"

#include <map>
#include <deque>
#include <functional>

MORTY_SPACE_BEGIN

class MEntity;
class PropertyBase;
class PropertyView : public BaseWidget
{
public:
	PropertyView();
    ~PropertyView() override;
	
	void Render() override;

	void Initialize(MainEditor* pMainEditor) override;
	void Release() override;
    
protected:

	void EditEntity(MEntity* pEntity);
	void UpdatePropertyList(MEntity* pEntity);

private:

	MEntity* m_pEntity = nullptr;
	std::deque<PropertyBase*> m_vPropertyList;
	
	std::map<MString, std::function<PropertyBase*()>> m_tCreatePropertyFactory;
};

MORTY_SPACE_END