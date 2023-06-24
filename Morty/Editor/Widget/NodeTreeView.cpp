#include "NodeTreeView.h"

#include "Scene/MScene.h"
#include "Scene/MEntity.h"
#include "Engine/MEngine.h"
#include "Object/MObject.h"
#include "Scene/MEntity.h"
#include "System/MObjectSystem.h"
#include "Component/MSceneComponent.h"

#include "imgui.h"
#include "Utility/SelectionEntityManager.h"


NodeTreeView::NodeTreeView()
	: IBaseView()
	, m_pScene(nullptr)
{
	m_strViewName = "Node Tree";
}

NodeTreeView::~NodeTreeView()
{

}

void NodeTreeView::SetScene(MScene* pScene)
{
	m_pScene = pScene;
}

void NodeTreeView::Render()
{
	if (!m_pScene)
		return;

	MObjectSystem* pObjectSystem = m_pEngine->FindSystem<MObjectSystem>();

	auto vEntity = m_pScene->GetAllEntity();
	for(MEntity* pEntity : vEntity)
	{
		MSceneComponent* pSceneComponent = pEntity->GetComponent<MSceneComponent>();
		if (!pSceneComponent || !pSceneComponent->GetParent())
		{
			//ImGui::Unindent(ImGui::GetTreeNodeToLabelSpacing());
			RenderNode(pEntity);
			//ImGui::Indent(ImGui::GetTreeNodeToLabelSpacing());
		}
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

void NodeTreeView::RenderNode(MEntity* pNode)
{
	if (!pNode) return;

	MScene* pScene = pNode->GetScene();
	MSceneComponent* pSceneComponent = pNode->GetComponent<MSceneComponent>();



	ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_FramePadding;
	if (!pSceneComponent || pSceneComponent->GetChildrenComponent().size() == 0)
		node_flags |= ImGuiTreeNodeFlags_Leaf;
// 	else if (pNode->GetID() == m_unRootNodeID)
// 		node_flags |= ImGuiTreeNodeFlags_DefaultOpen;

	if (SelectionEntityManager::GetInstance()->GetSelectedEntity() == pNode)
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
		SelectionEntityManager::GetInstance()->SetSelectedEntity(m_pScene, pNode);
	}
	if (bOpened)
	{
		if (pSceneComponent)
		{
			for (const auto& child : pSceneComponent->GetChildrenComponent())
			{
				MComponent* pComponent = pScene->GetComponent(child);
				RenderNode(pComponent->GetEntity());
			}
		}
		
		ImGui::TreePop();
	}
}

