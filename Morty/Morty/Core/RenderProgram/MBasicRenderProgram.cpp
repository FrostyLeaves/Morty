#include "MBasicRenderProgram.h"
#include "MEngine.h"
#include "MMaterial.h"
#include "MSkeleton.h"
#include "MViewport.h"
#include "MIRenderer.h"
#include "MResourceManager.h"
#include "Texture/MTextureResource.h"
#include "Model/MIMeshInstance.h"

M_OBJECT_IMPLEMENT(MBasicRenderProgram, MIRenderProgram)


MBasicRenderProgram::MBasicRenderProgram()
	: MIRenderProgram()
	, m_TransparentDrawMesh(false)
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

void MBasicRenderProgram::Render(MIRenderer* pRenderer, const std::vector<MViewport*>& vViewports)
{
// 	if (vViewports.empty())
// 		return;
// 
// 	MViewport* pViewport = vViewports[0];
// 
// 	pRenderer->RenderBegin(pRenderTarget);
// 
// 	pRenderer->BeginRenderPass(pRenderTarget);
// 
// 	pRenderer->SetViewport(pViewport->GetLeft(), pViewport->GetTop(), pViewport->GetWidth(), pViewport->GetHeight(), 0.0f, 1.0f);
// 
// 	std::vector<MShaderConstantParam*>& params = *m_pMaterial->GetShaderParams();
// 	if (!params.empty())
// 	{
// 		MShaderConstantParam* p1 = params[1];
// 		MStruct& srt1 = *p1->var.GetStruct();
// 
// 		if (float* pTime = srt1.FindMember<float>("time"))
// 			*pTime += 0.02f;
// 
// 		p1->SetDirty();
// 
// 		MShaderConstantParam* param = params[0];
// 		MStruct& srt = *param->var.GetStruct();
// 
// 		if (Vector2* pSize = srt.FindMember<Vector2>("screenSize"))
// 			*pSize = pViewport->GetSize();
// 
// 		param->SetDirty();
// 	}
// 
// 	pRenderer->SetUseMaterial(m_pMaterial);
// 
// 
// 	pRenderer->DrawMesh(&m_TransparentDrawMesh);
// 
// 	pRenderer->EndRenderPass(pRenderTarget);
// 
// 	pRenderer->RenderEnd(pRenderTarget);
}

void MBasicRenderProgram::OnCreated()
{
	Super::OnCreated();

	MResource* pVertixShader = GetEngine()->GetResourceManager()->LoadResource("./Shader/basic_test.mvs");
	MResource* pPixelShader = GetEngine()->GetResourceManager()->LoadResource("./Shader/basic_test.mps");

	MResource* pTextureRes = GetEngine()->GetResourceManager()->LoadResource("./Texture/test.png");

	m_pMaterial = GetEngine()->GetResourceManager()->CreateResource<MMaterial>();
	m_pMaterial->AddRef();

	m_pMaterial->LoadVertexShader(pVertixShader);
	m_pMaterial->LoadPixelShader(pPixelShader);

	std::vector<MShaderTextureParam*>& params = m_pMaterial->GetMaterialParamSet()->m_vTextures;
	if (!params.empty())
	{
		if (MTextureResource* pTextureResource = pTextureRes->DynamicCast<MTextureResource>())
		{
			MShaderTextureParam* p1 = params[0];
			p1->pTexture = pTextureResource->GetTextureTemplate();
			p1->SetDirty();
		}
	}

}

void MBasicRenderProgram::OnDelete()
{
	m_TransparentDrawMesh.DestroyBuffer(m_pEngine->GetDevice());
	m_pMaterial->SubRef();
	m_pMaterial = nullptr;

	Super::OnDelete();
}

void MBasicRenderProgram::DrawMeshInstance(MIRenderer* pRenderer, MIMeshInstance* pMeshInstance)
{
	pRenderer->DrawMesh(pMeshInstance->GetMesh());
}
