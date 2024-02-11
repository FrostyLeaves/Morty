#pragma once

#include "Main/BaseWidget.h"

#include "Math/Vector.h"
#include "Utility/MColor.h"
#include "Utility/MString.h"
#include "Utility/MTransform.h"
#include "Property/PropertyBase.h"

#include <map>
#include <deque>
#include <functional>

MORTY_SPACE_BEGIN

class MRenderGraphSetting;
class RenderSettingView : public BaseWidget
{
public:
	RenderSettingView();
    ~RenderSettingView() = default;

	void SetRenderGraph(std::shared_ptr<MRenderGraphSetting> pSettings);
	
	void Render() override;

	void Initialize(MainEditor* pMainEditor) override;
	void Release() override;
    
private:

	PropertyBase m_property;
	std::shared_ptr<MRenderGraphSetting> m_pSettings = nullptr;
	std::vector<MStringId> m_vOrderedNames;
};

MORTY_SPACE_END