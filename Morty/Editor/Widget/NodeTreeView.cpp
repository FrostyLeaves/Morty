#include "NodeTreeView.h"
#include "Component/MRenderMeshComponent.h"
#include "Component/MSceneComponent.h"
#include "Engine/MEngine.h"
#include "Main/MainEditor.h"
#include "Scene/MEntity.h"
#include "Scene/MScene.h"
#include "System/MObjectSystem.h"
#include "Utility/SelectionContext.h"
#include "Utility/SelectionManager.h"
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
        auto* sceneComponent = pEntity->GetComponent<MSceneComponent>();
        if (!sceneComponent || !sceneComponent->GetParent())
        {
            //ImGui::Unindent(ImGui::GetTreeNodeToLabelSpacing());
            RenderNode(pEntity);
            //ImGui::Indent(ImGui::GetTreeNodeToLabelSpacing());
        }
    }
}

void NodeTreeView::Initialize(MainEditor* pMainEditor) { BaseWidget::Initialize(pMainEditor); }

void NodeTreeView::Release() {}

void NodeTreeView::RenderNode(MEntity* node)
{
    if (!node) return;

    MScene*            scene           = node->GetScene();
    auto*              sceneComponent = node->GetComponent<MSceneComponent>();


    ImGuiTreeNodeFlags node_flags =
            ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_FramePadding;
    if (!sceneComponent || sceneComponent->GetChildrenComponent().empty()) node_flags |= ImGuiTreeNodeFlags_Leaf;
    if (SelectionContext::GetInstance()->GetSelectedEntity() == node) node_flags |= ImGuiTreeNodeFlags_Selected;


    bool bOpened = ImGui::TreeNodeEx(node, node_flags, "%s", node->GetName().c_str());
    if (ImGui::BeginPopupContextItem())
    {
        if (ImGui::Selectable("Delete")) { node->DeleteSelf(); }
        ImGui::EndPopup();
    }

    if (ImGui::IsItemClicked())
    {
        SelectionContext::GetInstance()->SetSelectedEntity(node);
        // Broadcast selection to all PropertyView panels
        SelectionManager::GetInstance()->BroadcastSelection(Selection(node));
    }
    if (bOpened)
    {
        if (sceneComponent)
        {
            for (const auto& child: sceneComponent->GetChildrenComponent())
            {
                MComponent* component = scene->GetComponent(child);
                RenderNode(component->GetEntity());
            }
        }

        ImGui::TreePop();
    }
}
