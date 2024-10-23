#pragma once

#include "Main/BaseWidget.h"

#include "Math/Vector.h"
#include "Utility/MColor.h"
#include "Utility/MString.h"
#include "Utility/MStringId.h"
#include "Utility/MTransform.h"

#include <deque>
#include <functional>
#include <map>

namespace morty
{

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
    MEntity*                                            m_entity = nullptr;
    std::deque<PropertyBase*>                           m_propertyList;

    std::map<MStringId, std::function<PropertyBase*()>> m_createPropertyFactory;
};

}// namespace morty