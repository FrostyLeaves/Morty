#include "NodeTreeView.h"

#include "MEngine.h"
#include "MObject.h"

#include "imgui.h"

#include "MNode.h"

NodeTreeView::NodeTreeView()
	: IBaseView()
	, m_unRootNodeID(MGlobal::M_INVALID_INDEX)
	, m_unSelectedObjectID(0)
{

}

NodeTreeView::~NodeTreeView()
{

}

void NodeTreeView::SetRootNode(MNode* pNode)
{
	if (pNode)
		m_unRootNodeID = pNode->GetObjectID();
	else
		m_unRootNodeID = MGlobal::M_INVALID_INDEX;
}

void NodeTreeView::Render()
{
	if(MNode* pRootNode = m_pEngine->GetObjectManager()->FindObject(m_unRootNodeID)->DynamicCast<MNode>())
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

MNode* NodeTreeView::GetSelectionNode()
{
	if (MObject* pObject = m_pEngine->GetObjectManager()->FindObject(m_unSelectedObjectID))
	{
		return pObject->DynamicCast<MNode>();
	}

	return nullptr;
}

void NodeTreeView::RenderNode(MNode* pNode)
{
	ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_FramePadding;
	if (pNode->GetChildren().size() + pNode->GetProtectedChildren().size() == 0)
		node_flags |= ImGuiTreeNodeFlags_Leaf;
	else if (pNode->GetObjectID() == m_unRootNodeID)
		node_flags |= ImGuiTreeNodeFlags_DefaultOpen;

	if (m_unSelectedObjectID == pNode->GetObjectID())
		node_flags |= ImGuiTreeNodeFlags_Selected;


	bool bOpened = ImGui::TreeNodeEx(pNode, node_flags, pNode->GetName().c_str());
	if (ImGui::BeginPopupContextItem())
	{
		if (ImGui::Selectable("Delete"))
		{
			pNode->DeleteLater();
		}
		ImGui::EndPopup();
	}

	if (ImGui::IsItemClicked())
	{
		m_unSelectedObjectID = pNode->GetObjectID();
	}
	if (bOpened)
	{
		for (MNode* pChild : pNode->GetChildren())
			RenderNode(pChild);

		for (MNode* pChild : pNode->GetProtectedChildren())
			RenderNode(pChild);

		ImGui::TreePop();
	}
}

