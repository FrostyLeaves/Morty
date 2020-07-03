#include "MBasicRenderProgram.h"
#include "MEngine.h"
#include "MMaterial.h"
#include "MSkeleton.h"
#include "MViewport.h"
#include "MIRenderer.h"
#include "MResourceManager.h"
#include "Model/MIMeshInstance.h"

M_OBJECT_IMPLEMENT(MBasicRenderProgram, MIRenderProgram)


MBasicRenderProgram::MBasicRenderProgram()
	: MIRenderProgram()
	, m_TransparentDrawMesh(true)
{
	MMesh<Vector2>& mesh = m_TransparentDrawMesh;
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
}

MBasicRenderProgram::~MBasicRenderProgram()
{
}

void MBasicRenderProgram::Render(MIRenderer* pRenderer, MViewport* pViewport, MScene* pScene, MIRenderTarget* pRenderTarget)
{
	pRenderer->SetViewport(pViewport->GetLeft(), pViewport->GetTop(), pViewport->GetWidth(), pViewport->GetHeight(), 0.0f, 1.0f);

	pRenderer->SetUseMaterial(m_pMaterial);


	pRenderer->DrawMesh(&m_TransparentDrawMesh);
}

void MBasicRenderProgram::OnCreated()
{
	Super::OnCreated();

	MResource* pVertixShader = GetEngine()->GetResourceManager()->LoadResource("./Shader/basic_test.mvs");
	MResource* pPixelShader = GetEngine()->GetResourceManager()->LoadResource("./Shader/basic_test.mps");


	m_pMaterial = GetEngine()->GetResourceManager()->CreateResource<MMaterial>();

	m_pMaterial->LoadVertexShader(pVertixShader);
	m_pMaterial->LoadPixelShader(pPixelShader);
}

void MBasicRenderProgram::OnDelete()
{
	m_TransparentDrawMesh.DestroyBuffer(m_pEngine->GetDevice());

	Super::OnDelete();
}

void MBasicRenderProgram::DrawMeshInstance(MIRenderer*& pRenderer, MIMeshInstance*& pMeshInstance, MShaderParam*& pMeshMatrixParam, MShaderParam*& pAnimationParam)
{
	Matrix4 worldTrans = pMeshInstance->GetWorldTransform();
	//Transposed and Inverse.
	Matrix3 matNormal(worldTrans.Transposed().Inverse(), 3, 3);

	MStruct& cStruct = *pMeshMatrixParam->var.GetStruct();
	cStruct[0] = worldTrans;
	cStruct[1] = matNormal;

	pMeshMatrixParam->SetDirty();
	pRenderer->SetShaderParam(*pMeshMatrixParam);

	if (pAnimationParam)
	{
		if (MSkeletonInstance* pSkeletonIns = pMeshInstance->GetSkeletonInstance())
		{
			MStruct& cAnimationStruct = *pAnimationParam->var.GetStruct();
			MVariantArray& cBonesArray = *cAnimationStruct[0].GetArray();

			const std::vector<MBone>& bones = pSkeletonIns->GetAllBones();
			uint32_t size = bones.size();
			if (size > MBONES_MAX_NUMBER) size = MBONES_MAX_NUMBER;

			for (uint32_t i = 0; i < size; ++i)
			{
				cBonesArray[i] = bones[i].m_matWorldTransform;
			}

			pAnimationParam->SetDirty();
			pRenderer->SetShaderParam(*pAnimationParam);

		}
	}

	pRenderer->DrawMesh(pMeshInstance->GetMesh());
}
