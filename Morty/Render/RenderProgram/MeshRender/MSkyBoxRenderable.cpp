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

void MSkyBoxRenderable::SetMesh(MIMesh* pMesh)
{
	m_pMesh = pMesh;
}

void MSkyBoxRenderable::SetMaterial(const std::shared_ptr<MMaterial>& pMaterial)
{
	m_pMaterial = pMaterial;
}

void MSkyBoxRenderable::SetFramePropertyBlockAdapter(const std::shared_ptr<IPropertyBlockAdapter>& pAdapter)
{
	m_pFramePropertyAdapter = pAdapter;
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

	const auto property = m_pFramePropertyAdapter->GetPropertyBlock();
	
	pCommand->SetShaderPropertyBlock(property);
	pCommand->DrawMesh(m_pMesh);

}
