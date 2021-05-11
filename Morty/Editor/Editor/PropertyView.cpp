#include "PropertyView.h"

#include "imgui.h"

#include "MObject.h"

#include "PropertyMSceneComponent.h"
#include "PropertyMCameraComponent.h"
#include "PropertyMSpotLight.h"
#include "PropertyMPointLight.h"
#include "PropertyMDirectionalLightComponent.h"
#include "PropertyMModelComponent.h"
#include "PropertyMRenderableMeshComponent.h"

#define REGISTER_PROPERTY( CLASS_NAME ) \
	m_tCreatePropertyFactory[#CLASS_NAME] = []() {return new Property##CLASS_NAME(); };

PropertyView::PropertyView()
	: IBaseView()
	, m_pEditorNode(nullptr)
	, m_vPropertyList()
{
	REGISTER_PROPERTY(MSceneComponent);
	REGISTER_PROPERTY(MCameraComponent);
	REGISTER_PROPERTY(MSpotLightComponent);
	REGISTER_PROPERTY(MPointLightComponent);
	REGISTER_PROPERTY(MDirectionalLightComponent);
	REGISTER_PROPERTY(MModelComponent);
	REGISTER_PROPERTY(MRenderableMeshComponent);
}

PropertyView::~PropertyView()
{
	for (PropertyBase* pPropertyBase : m_vPropertyList)
	{
		if(pPropertyBase)
			delete pPropertyBase;
	}

	m_vPropertyList.clear();
}

void PropertyView::SetEditorNode(MNode* pNode)
{
	if (m_pEditorNode == pNode)
		return;

	if (m_pEditorNode = pNode)
	{
		CreatePropertyList(pNode);
	}
}

void PropertyView::Render()
{
	if (nullptr == m_pEditorNode)
		return;

	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
	ImGui::Columns(2);
	ImGui::Separator();
	
	unsigned int unID = 0;

	EditNode(m_pEditorNode);

	ImGui::Columns(1);
	ImGui::Separator();
	ImGui::PopStyleVar();

}

void PropertyView::Initialize(MEngine* pEngine)
{

}

void PropertyView::Release()
{

}

void PropertyView::Input(MInputEvent* pEvent)
{

}

void PropertyView::EditNode(MNode* pNode)
{
	if (pNode)
	{
		for (PropertyBase* pPropertyBase : m_vPropertyList)
		{
			if (pPropertyBase)
			{
				pPropertyBase->EditNode(pNode);
			}
		}
	}
}

void PropertyView::CreatePropertyList(MNode* pNode)
{
	for (PropertyBase* pPropertyBase : m_vPropertyList)
	{
		if(pPropertyBase)
			delete pPropertyBase;
	}
	m_vPropertyList.clear();

	MString name = pNode->GetTypeName();


	auto& vComponents = pNode->GetComponents();
	for(MComponent* pComponent : vComponents)
	{
		if (auto func = m_tCreatePropertyFactory[pComponent->GetTypeIdentifier()->m_strName])
		{
			m_vPropertyList.push_front(func());
		}
	}
}
