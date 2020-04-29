#include "ResourceView.h"

#include "imgui.h"

#include "MEngine.h"
#include "MResource.h"
#include "MResourceManager.h"

ResourceView::ResourceView()
	: IBaseView()
	, m_pEngine(nullptr)
{

}

ResourceView::~ResourceView()
{

}

void ResourceView::Render()
{
	static const char* vResourceType[] = {
		"Default",
		"Model",
		"Shader",
		"Material",
		"Texture"
	};


	ImGui::Columns(4);

	MResourceManager* pResManager = m_pEngine->GetResourceManager();

	std::map<MResourceID, MResource*>& resources = *pResManager->GetAllResources();
	int ITEMS_COUNT = resources.size();
	ImGuiListClipper clipper(ITEMS_COUNT - 1);  // Also demonstrate using the clipper for large list
	
	while (clipper.Step())
	{
		auto iter = resources.begin();
		for (int i = 0; i < clipper.DisplayStart; ++i)
		{
			++iter;
		}

		for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
		{
			MResource* pResource = iter->second;
			ImGui::Text("%lu", pResource->GetResourceID());
			ImGui::NextColumn();
			ImGui::Text(vResourceType[pResource->GetType()]);
			ImGui::NextColumn();
			ImGui::Text(pResource->GetResourcePath().c_str());
			ImGui::NextColumn();
			ImGui::Text("%u", pResource->GetRefNumber());
			ImGui::NextColumn();

			++iter;
		}
	}
	ImGui::Columns(1);
}

void ResourceView::Initialize(MEngine* pEngine)
{
	m_pEngine = pEngine;
}

void ResourceView::Release()
{

}

void ResourceView::Input(MInputEvent* pEvent)
{

}

