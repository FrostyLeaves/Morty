#include "MBasicRenderSystem.h"
#include "MEngine.h"
#include "MResourceManager.h"
#include "MIRenderer.h"

#include "MScene.h"
#include "MViewport.h"
#include "MTexture.h"
#include "MShadowTextureRenderTarget.h"
#include "MTransparentRenderTarget.h"

#include "MPainter.h"
#include "MTexture.h"
#include "MRenderStructure.h"
#include "MTransformCoord.h"

#include "MCamera.h"
#include "MSkyBox.h"
#include "MSkeleton.h"
#include "Model/MModelInstance.h"
#include "Model/MIMeshInstance.h"
#include "Model/MIModelMeshInstance.h"
#include "Light/MSpotLight.h"
#include "Light/MPointLight.h"
#include "Light/MDirectionalLight.h"

#include "MSkeleton.h"
#include "Material/MMaterialResource.h"
#include "Model/MMeshResource.h"
#include "Model/MModelResource.h"

#include <algorithm>


M_OBJECT_IMPLEMENT(MBasicRenderSystem, MISystem)


MBasicRenderSystem::MBasicRenderSystem()
	: MISystem()
	, m_DrawMesh(true)
	, m_pDrawMaterial(nullptr)
{
	
}

MBasicRenderSystem::~MBasicRenderSystem()
{
}

void MBasicRenderSystem::Tick(const float& fDelta)
{

}

void MBasicRenderSystem::Render(MIRenderer* pRenderer, MViewport* pViewport, MScene* pScene, MIRenderTarget* pRenderTarget)
{
	pRenderer->SetUseMaterial(m_pDrawMaterial, true);

	pRenderer->DrawMesh(&m_DrawMesh);
}

void MBasicRenderSystem::OnCreated()
{
	MMesh<Vector2>& mesh = m_DrawMesh;
	mesh.ResizeVertices(4);
	Vector2* vVertices = (Vector2*)mesh.GetVertices();

	vVertices[0] = Vector2(-1, -1);
	vVertices[1] = Vector2(1, -1);
	vVertices[2] = Vector2(-1, 1);
	vVertices[3] = Vector2(1, 1);

	mesh.ResizeIndices(2, 3);
	uint32_t* vIndices = mesh.GetIndices();

	vIndices[0] = 0;
	vIndices[1] = 2;
	vIndices[2] = 1;

	vIndices[3] = 2;
	vIndices[4] = 3;
	vIndices[5] = 1;


	if (MResource* pMaterialRes = m_pEngine->GetResourceManager()->LoadResource(""))
	{
		m_pDrawMaterial = pMaterialRes->DynamicCast<MMaterial>();
	}
}

void MBasicRenderSystem::OnDelete()
{
	m_DrawMesh.DestroyBuffer(m_pEngine->GetDevice());
}
