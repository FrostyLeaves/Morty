#include "MForwardRenderProgram.h"

#include "MScene.h"
#include "MEngine.h"
#include "MTexture.h"
#include "MViewport.h"
#include "MFunction.h"

#include "MSceneComponent.h"
#include "MRenderableMeshComponent.h"

MORTY_CLASS_IMPLEMENT(MForwardRenderProgram, MTaskNode)

MForwardRenderProgram::MForwardRenderProgram()
	: MTaskNode()
{
	
}

MForwardRenderProgram::~MForwardRenderProgram()
{
}

void MForwardRenderProgram::Run(MTaskNode* pTaskNode)
{
	MViewport* pViewport = nullptr;

	GenerateRenderGroup(pViewport);

}

void MForwardRenderProgram::Initialize()
{
	BindTaskFunction(M_CLASS_FUNCTION_BIND_1(MForwardRenderProgram::Run, this));
}

void MForwardRenderProgram::Release()
{
	
}

void MForwardRenderProgram::GenerateRenderGroup(MViewport* pViewport)
{
	MScene* pScene = pViewport->GetScene();

	Vector3 v3BoundsMin(+FLT_MAX, +FLT_MAX, +FLT_MAX);
	Vector3 v3BoundsMax(-FLT_MAX, -FLT_MAX, -FLT_MAX);


	MComponentGroup<MRenderableMeshComponent>* pMeshComponents = pScene->FindComponents<MRenderableMeshComponent>();

	if (!pMeshComponents)
		return;

	m_tMaterialGroup.clear();

	for (MRenderableMeshComponent& meshComp : pMeshComponents->m_vComponent)
	{
		MMaterial* pMaterial = meshComp.GetMaterial();
		auto& meshes = m_tMaterialGroup[pMaterial];

		MSceneComponent* pSceneComponent = meshComp.GetEntity()->GetComponent<MSceneComponent>();

		if (!pSceneComponent->GetVisibleRecursively())
			continue;

		const MBoundsAABB* pBounds = meshComp.GetBoundsAABB();

		if (MCameraFrustum::EOUTSIDE == pViewport->GetCameraFrustum().ContainTest(*pBounds))
			continue;

		meshes.push_back(meshComp.GetMesh());

		pBounds->UnionMinMax(v3BoundsMin, v3BoundsMax);
	}

	cMeshRenderAABB.SetMinMax(v3BoundsMin, v3BoundsMax);
}
