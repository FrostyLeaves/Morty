#include "RenderSettingView.h"

#include "imgui.h"

#include "Object/MObject.h"
#include "RenderProgram/RenderGraph/MRenderGraphSetting.h"
#include "Utility/MFunction.h"

RenderSettingView::RenderSettingView()
	: BaseWidget()
{
	m_strViewName = "Render Setting";
}

void RenderSettingView::SetRenderGraph(std::shared_ptr<MRenderGraphSetting> pSettings)
{
	m_pSettings = pSettings;
	m_vOrderedNames.clear();

	if (!m_pSettings)
	{
		return;
	}

	for (auto& [name, _] : m_pSettings->GetSettings())
	{
		 UNION_ORDER_PUSH_BACK_VECTOR<MStringId>(m_vOrderedNames, name,
		     [](const MStringId& a, const MStringId& b) {
		        return a.ToString() < b.ToString();
			},
			[](const MStringId& a, const MStringId& b) {
				return a.ToString() == b.ToString();
			}
			);
	}
}

void RenderSettingView::Render()
{
	if (!m_pSettings)
	{
		return;
	}

	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
	ImGui::Columns(2);
	ImGui::Separator();

	for(const auto& name : m_vOrderedNames)
	{
		MVariant variant = m_pSettings->GetPropertyVariant(name);
		if (m_property.EditMVariant(name.ToString(), variant))
		{
			m_pSettings->MarkDirty(name);
		}
	}

	ImGui::Columns(1);
	ImGui::Separator();
	ImGui::PopStyleVar();

}

void RenderSettingView::Initialize(MainEditor* pMainEditor)
{
	BaseWidget::Initialize(pMainEditor);
}

void RenderSettingView::Release()
{

}
