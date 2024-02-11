#include "GuizmoWidget.h"


#include "Scene/MEntity.h"
#include "Engine/MEngine.h"
#include "Basic/MViewport.h"
#include "RenderProgram/MRenderInfo.h"
#include "Resource/MResource.h"
#include "System/MRenderSystem.h"
#include "System/MResourceSystem.h"

#include "Component/MSceneComponent.h"
#include "Component/MCameraComponent.h"
#include "Utility/SelectionEntityManager.h"

using namespace morty;

GuizmoWidget::GuizmoWidget()
	: BaseWidget()
{
	m_strViewName = "Guizmo";
}

void GuizmoWidget::Render()
{
	MEntity* pEntity = SelectionEntityManager::GetInstance()->GetSelectedEntity();
	if (nullptr == pEntity)
	{
		return;
	}

	MSceneComponent* pEditorSceneComponent = pEntity->GetComponent<MSceneComponent>();
	if (nullptr == pEditorSceneComponent)
	{
		return;
	}

	MViewport* pViewport = GetViewport();

	MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();
	MEntity* pCameraEntity = pViewport->GetCamera();
	MSceneComponent* pCameraSceneComponent = pCameraEntity->GetComponent<MSceneComponent>();
	MCameraComponent* pCameraComponent = pCameraEntity->GetComponent<MCameraComponent>();
	
	Matrix4 cameraView = pRenderSystem->GetCameraViewMatrix(pCameraSceneComponent).Transposed();
	Matrix4 cameraProjection = pRenderSystem->GetCameraProjectionMatrix(pCameraComponent, pViewport->GetWidth(), pViewport->GetHeight(), pCameraComponent->GetZNear(), pCameraComponent->GetZFar()).Transposed();
	Matrix4 editorMatrix = pEditorSceneComponent->GetWorldTransform().Transposed();
	
	ImGuizmo::DrawCubes((float*)cameraView.m, (float*)cameraProjection.m, (float*)editorMatrix.m, 1);
	ImGuizmo::Manipulate((float*)cameraView.m, (float*)cameraProjection.m, m_eGizmoOperation, m_eGizmoMode, (float*)editorMatrix.m);

	editorMatrix = (pEditorSceneComponent->GetParentWorldTransform().Inverse() * editorMatrix.Transposed());
	
	if (ImGuizmo::IsUsing())
	{
		//MTransform transform = editorMatrix;
		//transform.SetPosition(matrixTranslation);
		//transform.SetRotation(Quaternion::FromEuler(matrixRotation));
		//transform.SetScale(matrixScale);

		//pEditorSceneComponent->SetTransform(transform);

		MTransform transform = editorMatrix;
		pEditorSceneComponent->SetTransform(transform);
	}
}

void GuizmoWidget::Initialize(MainEditor* pMainEditor)
{
	BaseWidget::Initialize(pMainEditor);
}

void GuizmoWidget::Release()
{

}
