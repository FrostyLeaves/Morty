#include "MFloorRender.h"

#include "Scene/MScene.h"
#include "Engine/MEngine.h"
#include "Material/MMaterial.h"
#include "Render/MRenderCommand.h"

#include "System/MRenderSystem.h"
#include "System/MResourceSystem.h"

#include "Component/MSkyBoxComponent.h"

#include "Mesh/MMeshManager.h"
#include "Manager/MEnvironmentManager.h"

void MFloorRender::SetScene(MScene* pScene)
{
	m_pScene = pScene;
}

void MFloorRender::SetFramePropertyBlockAdapter(const std::shared_ptr<IPropertyBlockAdapter>& pAdapter)
{
	m_pFramePropertyAdapter = pAdapter;
}

void MFloorRender::Render(MIRenderCommand* pCommand)
{
	if (!m_pScene)
	{
		MORTY_ASSERT(m_pScene);
		return;
	}

	const MMeshManager* pMeshManager = m_pScene->GetEngine()->FindGlobalObject<MMeshManager>();

	/*
	const auto pMaterial = pEnvironment->GetMaterial();
	if (!pMaterial)
	{
		MORTY_ASSERT(pMaterial);
		return;
	}

	pCommand->SetUseMaterial(pMaterial);
	*/

	const auto vPropertyBlock = m_pFramePropertyAdapter->GetPropertyBlock();
	for (const auto& property : vPropertyBlock)
	{
		pCommand->SetShaderParamSet(property);
	}
	pCommand->DrawMesh(pMeshManager->GetScreenRect());

}
