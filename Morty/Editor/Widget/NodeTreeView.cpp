#include "NodeTreeView.h"
#include "Component/MRenderMeshComponent.h"
#include "Component/MSceneComponent.h"
#include "Engine/MEngine.h"
#include "Main/MainEditor.h"
#include "MaterialView.h"
#include "Scene/MEntity.h"
#include "Scene/MScene.h"
#include "System/MObjectSystem.h"
#include "Utility/SelectionEntityManager.h"
#include "imgui.h"

using namespace morty;

NodeTreeView::NodeTreeView()
    : BaseWidget()
{
    m_strViewName = "Node Tree";
}

void NodeTreeView::Render()
{
    if (!GetScene()) { return; }

    auto vEntity = GetScene()->GetAllEntity();
    for (MEntity* pEntity: vEntity)
    {
        auto* pSceneComponent = pEntity->GetComponent<MSceneComponent>();
        if (!pSceneComponent || !pSceneComponent->GetParent())
        {
            //ImGui::Unindent(ImGui::GetTreeNodeToLabelSpacing());
            RenderNode(pEntity);
            //ImGui::Indent(ImGui::GetTreeNodeToLabelSpacing());
        }
    }
}

void NodeTreeView::Initialize(MainEditor* pMainEditor) { BaseWidget::Initialize(pMainEditor); }

void NodeTreeView::Release() {}

void NodeTreeView::RenderNode(MEntity* pNode)
{
    if (!pNode) return;

    MScene*            scene           = pNode->GetScene();
    auto*              pSceneComponent = pNode->GetComponent<MSceneComponent>();


    ImGuiTreeNodeFlags node_flags =
            ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_FramePadding;
    if (!pSceneComponent || pSceneComponent->GetChildrenComponent().empty()) node_flags |= ImGuiTreeNodeFlags_Leaf;
    if (SelectionEntityManager::GetInstance()->GetSelectedEntity() == pNode) node_flags |= ImGuiTreeNodeFlags_Selected;


    bool bOpened = ImGui::TreeNodeEx(pNode, node_flags, "%s", pNode->GetName().c_str());
    if (ImGui::BeginPopupContextItem())
    {
        if (ImGui::Selectable("Delete")) { pNode->DeleteSelf(); }
        ImGui::EndPopup();
    }

    if (ImGui::IsItemClicked()) { SelectionEntityManager::GetInstance()->SetSelectedEntity(pNode); }
    if (bOpened)
    {
        if (pSceneComponent)
        {
            for (const auto& child: pSceneComponent->GetChildrenComponent())
            {
                MComponent* component = scene->GetComponent(child);
                RenderNode(component->GetEntity());
            }
        }

        ImGui::TreePop();
    }
}
