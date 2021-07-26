#include "NodeTreeView.h"

#include "MScene.h"
#include "MEngine.h"
#include "MObject.h"
#include "MEntity.h"
#include "MObjectSystem.h"
#include "MSceneComponent.h"

#include "imgui.h"


NodeTreeView::NodeTreeView()
	: IBaseView()
	, m_unRootNodeID(MGlobal::M_INVALID_INDEX)
	, m_unSelectedObjectID(0)
{

}

NodeTreeView::~NodeTreeView()
{

}

void NodeTreeView::SetRootNode(MEntity* pNode)
{
	if (pNode)
		m_unRootNodeID = pNode->GetID();
	else
		m_unRootNodeID = MGlobal::M_INVALID_INDEX;
}

void NodeTreeView::Render()
{
	MObjectSystem* pObjectSystem = m_pEngine->FindSystem<MObjectSystem>();

	if(MEntity* pRootNode = pObjectSystem->FindObject(m_unRootNodeID)->DynamicCast<MEntity>())
	{
		ImGui::Unindent(ImGui::GetTreeNodeToLabelSpacing());
		RenderNode(pRootNode);
		ImGui::Indent(ImGui::GetTreeNodeToLabelSpacing());
	}
}

void NodeTreeView::Initialize(MEngine* pEngine)
{
	m_pEngine = pEngine;
}

void NodeTreeView::Release()
{

}

void NodeTreeView::Input(MInputEvent* pEvent)
{

}

MEntity* NodeTreeView::GetSelectionNode()
{
	MObjectSystem* pObjectSystem = m_pEngine->FindSystem<MObjectSystem>();

	if (MObject* pObject = pObjectSystem->FindObject(m_unSelectedObjectID))
	{
		return pObject->DynamicCast<MEntity>();
	}

	return nullptr;
}

void NodeTreeView::RenderNode(MEntity* pNode)
{
	if (!pNode) return;

	MScene* pScene = pNode->GetScene();
	MSceneComponent* pSceneComponent = pNode->GetComponent<MSceneComponent>();

	auto vChildrenComponent = pSceneComponent->GetChildrenComponent();

	ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_FramePadding;
	if (vChildrenComponent.size() == 0)
		node_flags |= ImGuiTreeNodeFlags_Leaf;
	else if (pNode->GetID() == m_unRootNodeID)
		node_flags |= ImGuiTreeNodeFlags_DefaultOpen;

	if (m_unSelectedObjectID == pNode->GetID())
		node_flags |= ImGuiTreeNodeFlags_Selected;


	bool bOpened = ImGui::TreeNodeEx(pNode, node_flags, pNode->GetName().c_str());
	if (ImGui::BeginPopupContextItem())
	{
		if (ImGui::Selectable("Delete"))
		{
			pNode->DeleteSelf();
		}
		ImGui::EndPopup();
	}

	if (ImGui::IsItemClicked())
	{
		m_unSelectedObjectID = pNode->GetID();
	}
	if (bOpened)
	{
		for (const auto& child : vChildrenComponent)
		{
			MComponent* pComponent = pScene->GetComponent(child);
			RenderNode(pComponent->GetEntity());
		}
		
		ImGui::TreePop();
	}
}

