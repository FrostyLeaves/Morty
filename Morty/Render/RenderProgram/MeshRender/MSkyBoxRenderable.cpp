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

using namespace morty;

void MSkyBoxRenderable::SetMesh(MIMesh* pMesh)
{
	m_pMesh = pMesh;
}

void MSkyBoxRenderable::SetMaterial(const std::shared_ptr<MMaterial>& pMaterial)
{
	m_pMaterial = pMaterial;
}

void MSkyBoxRenderable::SetPropertyBlockAdapter(const std::vector<std::shared_ptr<IPropertyBlockAdapter>>& vAdapter)
{
	m_vFramePropertyAdapter = vAdapter;
}

void MSkyBoxRenderable::Render(MIRenderCommand* pCommand)
{
	if (!m_pMesh)
	{
		MORTY_ASSERT(m_pMesh);
		return;
	}

	if (!m_pMaterial)
	{
		MORTY_ASSERT(m_pMaterial);
		return;
	}

	pCommand->SetUseMaterial(m_pMaterial);

	for (const auto& pAdapter : m_vFramePropertyAdapter)
	{
		pCommand->SetShaderPropertyBlock(pAdapter->GetPropertyBlock());
	}

	pCommand->DrawMesh(m_pMesh);
}
