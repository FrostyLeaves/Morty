#include "MSkyBoxRenderable.h"

#include "Scene/MScene.h"
#include "Engine/MEngine.h"
#include "Material/MMaterial.h"
#include "Render/MRenderCommand.h"

#include "System/MRenderSystem.h"
#include "System/MResourceSystem.h"

#include "Component/MSkyBoxComponent.h"

#include "Mesh/MMeshManager.h"
#include "Manager/MEnvironmentManager.h"

void MSkyBoxRenderable::SetScene(MScene* pScene)
{
	m_pScene = pScene;
}

void MSkyBoxRenderable::SetFramePropertyBlockAdapter(const std::shared_ptr<IPropertyBlockAdapter>& pAdapter)
{
	m_pFramePropertyAdapter = pAdapter;
}

void MSkyBoxRenderable::Render(MIRenderCommand* pCommand)
{
	if (!m_pScene)
	{
		MORTY_ASSERT(m_pScene);
		return;
	}

	const MMeshManager* pMeshManager = m_pScene->GetEngine()->FindGlobalObject<MMeshManager>();
	const MEnvironmentManager* pEnvironment = m_pScene->GetManager<MEnvironmentManager>();

	if (!pEnvironment->HasEnvironmentComponent())
	{
		return;
	}

	const auto pMaterial = pEnvironment->GetMaterial();
	if (!pMaterial)
	{
		MORTY_ASSERT(pMaterial);
		return;
	}

	pCommand->SetUseMaterial(pMaterial);

	const auto property = m_pFramePropertyAdapter->GetPropertyBlock();
	
	pCommand->SetShaderPropertyBlock(property);
	pCommand->DrawMesh(pMeshManager->GetSkyBox());

}
