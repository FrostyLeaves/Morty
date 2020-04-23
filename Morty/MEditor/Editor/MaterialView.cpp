#include "MaterialView.h"
#include "MMaterial.h"


#include "imgui.h"

MaterialView::MaterialView()
	: m_pResource(nullptr)
	, m_pMaterial(nullptr)
{

}

MaterialView::~MaterialView()
{
	if (m_pResource)
	{
		SetMaterial(nullptr);
	}
	m_pMaterial = nullptr;
}

void MaterialView::SetMaterial(MMaterial* pMaterial)
{

	if (m_pResource)
	{
		if (m_pResource->GetResource() == pMaterial)
			return;
		else
		{
			delete m_pResource;
			m_pResource = nullptr;
			m_pMaterial = nullptr;
		}
	}

	m_pResource = new MResourceHolder(pMaterial);
	m_pMaterial = pMaterial;

}

void MaterialView::Render()
{

	if (m_pMaterial)
	{
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
		ImGui::Columns(2);
		ImGui::Separator();

		unsigned int unID = 0;

		m_propertyBase.EditMMaterial(m_pMaterial);

		ImGui::Columns(1);
		ImGui::Separator();
		ImGui::PopStyleVar();
	}
}
