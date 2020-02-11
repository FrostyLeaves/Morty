#include "NodeTreeView.h"

#include "imgui.h"

#include "MNode.h"

NodeTreeView::NodeTreeView()
	: m_pRootNode(nullptr)
	, m_unSelectedObjectID(0)
{

}

NodeTreeView::~NodeTreeView()
{

}

void NodeTreeView::SetRootNode(MNode* pNode)
{
	m_pRootNode = pNode;
}

void NodeTreeView::Render()
{
	if(m_pRootNode)
	{
		ImGui::Unindent(ImGui::GetTreeNodeToLabelSpacing());
		RenderNode(m_pRootNode);
		ImGui::Indent(ImGui::GetTreeNodeToLabelSpacing());
	}
}

MObject* NodeTreeView::GetSelectionNode()
{
	if (m_pRootNode)
	{
		if (MObject * pObject = m_pRootNode->GetObjectManager()->FindObject(m_unSelectedObjectID))
		{
			return pObject;
		}
	}

	return nullptr;
}

void NodeTreeView::RenderNode(MNode* pNode)
{
	ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
	if (pNode->GetChildren().size() + pNode->GetFixedChildren().size() == 0)
		node_flags |= ImGuiTreeNodeFlags_Leaf;

	if (m_unSelectedObjectID == pNode->GetObjectID())
		node_flags |= ImGuiTreeNodeFlags_Selected;

	bool bOpened = ImGui::TreeNodeEx(pNode, node_flags, pNode->GetName().c_str());
	if (ImGui::IsItemClicked())
	{
		m_unSelectedObjectID = pNode->GetObjectID();
	}
	if (bOpened)
	{
		for (MNode* pChild : pNode->GetChildren())
			RenderNode(pChild);

		for (MNode* pChild : pNode->GetFixedChildren())
			RenderNode(pChild);

		ImGui::TreePop();
	}
}

