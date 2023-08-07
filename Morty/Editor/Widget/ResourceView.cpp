#include "ResourceView.h"

#include "imgui.h"

#include "Engine/MEngine.h"
#include "Resource/MResource.h"
#include "System/MResourceSystem.h"

ResourceView::ResourceView()
	: BaseWidget()
{
	m_strViewName = "Resource";
}

void ResourceView::Render()
{
	static const char* vResourceType[] = {
		"Default",
		"Shader",
		"Material",
		"Texture",
		"Node",
		"Mesh",
		"Skeleton",
		"SkelAnim"
	};


	ImGui::Columns(4);

	MResourceSystem* pResourceSystem = GetEngine()->FindSystem<MResourceSystem>();

	std::map<MResourceID, std::shared_ptr<MResource>>& resources = *pResourceSystem->GetAllResources();
	int ITEMS_COUNT = resources.size();
	ImGuiListClipper clipper(ITEMS_COUNT);  // Also demonstrate using the clipper for large list
	
	while (clipper.Step())
	{
		auto iter = resources.begin();
		for (int i = 0; i < clipper.DisplayStart; ++i)
		{
			++iter;
		}

		for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
		{
			std::shared_ptr<MResource> pResource = iter->second;
			ImGui::Text("%lu", pResource->GetResourceID());
			ImGui::NextColumn();
			ImGui::Text("%s", pResource->GetTypeName().c_str());
			ImGui::NextColumn();
			ImGui::Text("%s", pResource->GetResourcePath().c_str());
			ImGui::NextColumn();
			ImGui::Text("%u", pResource.use_count());
			ImGui::NextColumn();

			++iter;
		}
	}
	ImGui::Columns(1);
}

void ResourceView::Initialize(MainEditor* pMainEditor)
{
	BaseWidget::Initialize(pMainEditor);
}

void ResourceView::Release()
{

}

void ResourceView::Input(MInputEvent* pEvent)
{

}

