#include "PropertyView.h"

#include "imgui.h"

#include "MObject.h"
#include "M3DNode.h"

#include "PropertyM3DNode.h"
#include "PropertyMCamera.h"
#include "PropertyMPointLight.h"
#include "PropertyMSpotLight.h"
#include "PropertyMDirectionalLight.h"
#include "PropertyMModelInstance.h"
#include "PropertyMIModelMeshInstance.h"

#define REGISTER_PROPERTY( CLASS_NAME ) \
	m_tCreatePropertyFactory[#CLASS_NAME] = []() {return new Property##CLASS_NAME(); };

PropertyView::PropertyView()
	: IBaseView()
	, m_pEditorObject(nullptr)
	, m_vPropertyList()
{
	REGISTER_PROPERTY(M3DNode);
	REGISTER_PROPERTY(MCamera);
	REGISTER_PROPERTY(MPointLight);
	REGISTER_PROPERTY(MSpotLight);
	REGISTER_PROPERTY(MDirectionalLight);
	REGISTER_PROPERTY(MModelInstance);
	REGISTER_PROPERTY(MIModelMeshInstance);
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

void PropertyView::SetEditorObject(MObject* pObject)
{
	if (m_pEditorObject == pObject)
		return;

	m_pEditorObject = pObject;

	CreatePropertyList(pObject);
}

void PropertyView::Render()
{
	if (nullptr == m_pEditorObject)
		return;

	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
	ImGui::Columns(2);
	ImGui::Separator();
	
	unsigned int unID = 0;

	EditObject(m_pEditorObject);

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

void PropertyView::EditObject(MObject* pObject)
{
	for (PropertyBase* pPropertyBase : m_vPropertyList)
	{
		if (pPropertyBase)
		{
			pPropertyBase->EditObject(pObject);
		}
	}
}

void PropertyView::CreatePropertyList(MObject* pObject)
{
	for (PropertyBase* pPropertyBase : m_vPropertyList)
	{
		if(pPropertyBase)
			delete pPropertyBase;
	}
	m_vPropertyList.clear();

	MTypeIdentifierConstPointer pointer = pObject->GetTypeIdentifier();
	MString name = pObject->GetObjectClassName();

	while (pointer)
	{
		if (auto func = m_tCreatePropertyFactory[pointer->m_strName])
		{
			m_vPropertyList.push_front(func());
		}

		pointer = pointer->m_pBaseTypeIdentifier;
	}
}
