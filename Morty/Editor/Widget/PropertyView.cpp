#include "PropertyView.h"

#include "imgui.h"

#include "Object/MObject.h"

#include "Property/PropertyMSceneComponent.h"
#include "Property/PropertyMCameraComponent.h"
#include "Property/PropertyMSpotLight.h"
#include "Property/PropertyMPointLight.h"
#include "Property/PropertyMDirectionalLightComponent.h"
#include "Property/PropertyMModelComponent.h"
#include "Property/PropertyMRenderMeshComponent.h"
#include "Utility/SelectionEntityManager.h"

#define REGISTER_PROPERTY( CLASS_NAME ) \
	m_tCreatePropertyFactory[#CLASS_NAME] = []() {return new Property##CLASS_NAME(); };

PropertyView::PropertyView()
	: IBaseView()
	, m_vPropertyList()
{
	m_strViewName = "Property";

	REGISTER_PROPERTY(MSceneComponent);
	REGISTER_PROPERTY(MCameraComponent);
	REGISTER_PROPERTY(MSpotLightComponent);
	REGISTER_PROPERTY(MPointLightComponent);
	REGISTER_PROPERTY(MDirectionalLightComponent);
	REGISTER_PROPERTY(MModelComponent);
	REGISTER_PROPERTY(MRenderMeshComponent);
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

void PropertyView::Render()
{
	MEntity* pEntity = SelectionEntityManager::GetInstance()->GetSelectedEntity();
	if (nullptr == pEntity)
		return;

	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
	ImGui::Columns(2);
	ImGui::Separator();
	
	unsigned int unID = 0;

	EditEntity(pEntity);

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

void PropertyView::EditEntity(MEntity* pNode)
{
	if (pNode)
	{
		for (PropertyBase* pPropertyBase : m_vPropertyList)
		{
			if (pPropertyBase)
			{
				pPropertyBase->EditEntity(pNode);
			}
		}
	}
}

void PropertyView::CreatePropertyList(MEntity* pNode)
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
		if (auto func = m_tCreatePropertyFactory[pComponent->GetType()->m_strName])
		{
			m_vPropertyList.push_back(func());
		}
	}
}
