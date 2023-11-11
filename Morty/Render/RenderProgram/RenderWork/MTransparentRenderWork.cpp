#include "MTransparentRenderWork.h"

#include "Engine/MEngine.h"
#include "Basic/MViewport.h"
#include "Model/MSkeleton.h"
#include "Model/MSkeletonInstance.h"
#include "Render/MRenderCommand.h"

#include "System/MRenderSystem.h"
#include "System/MResourceSystem.h"

#include "Resource/MTextureResource.h"
#include "Resource/MMaterialResource.h"

#include "Component/MRenderMeshComponent.h"

#include "Mesh/MMeshManager.h"
#include "RenderProgram/MeshRender/MCullingResultRenderable.h"
#include "Resource/MTextureResourceUtil.h"
#include "Utility/MGlobal.h"

MORTY_CLASS_IMPLEMENT(MTransparentRenderWork, ISinglePassRenderWork)



class MORTY_API MTransparentPropertyDecorator : public IShaderPropertyUpdateDecorator
{
public:
	void BindMaterial(const std::shared_ptr<MShaderPropertyBlock>& pShaderPropertyBlock) override
	{
		MORTY_ASSERT(m_pTransparentFrontTextureParam = pShaderPropertyBlock->FindTextureParam(MShaderPropertyName::TRANSPARENT_TEXTURE_INPUT_0));
		MORTY_ASSERT(m_pTransparentBackTextureParam = pShaderPropertyBlock->FindTextureParam(MShaderPropertyName::TRANSPARENT_TEXTURE_INPUT_1));
	}

	void Update(const MRenderInfo& info) override
	{
		MORTY_UNUSED(info);
	}

	std::shared_ptr<MShaderTextureParam> m_pTransparentFrontTextureParam = nullptr;
	std::shared_ptr<MShaderTextureParam> m_pTransparentBackTextureParam = nullptr;
};

void MTransparentRenderWork::Initialize(MEngine* pEngine)
{
	Super::Initialize(pEngine);

	InitializeTexture();
	InitializeMaterial();
	InitializeFrameShaderParams();


	InitializePeelRenderPass();
	InitializeFillRenderPass();
}

void MTransparentRenderWork::Release(MEngine* pEngine)
{
	MORTY_UNUSED(pEngine);
	
	ReleaseFillRenderPass();
	ReleasePeelRenderPass();

	ReleaseFrameShaderParams();
	ReleaseMaterial();
	ReleaseTexture();

	Super::Release(pEngine);
}

void MTransparentRenderWork::Resize(Vector2i size)
{
	Super::Resize(size);

	MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();

	if (m_peelRenderPass.GetFrameBufferSize() != size)
	{
		pRenderSystem->ResizeFrameBuffer(m_peelRenderPass, size);
	}

}

void MTransparentRenderWork::Render(MRenderInfo& info, const std::vector<MCullingResultRenderable*>& vRenderable)
{

	RenderDepthPeel(info, vRenderable);
	RenderScreenFill(info);
	
}

void MTransparentRenderWork::RenderDepthPeel(MRenderInfo& info, const std::vector<MCullingResultRenderable*>& vRenderable)
{
	MIRenderCommand* pCommand = info.pPrimaryRenderCommand;
	if (!pCommand)
	{
		MORTY_ASSERT(pCommand);
		return;
	}

	MMeshManager* pMeshManager = GetEngine()->FindGlobalObject<MMeshManager>();
	if (!pMeshManager)
	{
		MORTY_ASSERT(pMeshManager);
		return;
	}

	const auto pDepthTexture = GetDepthTexture();

	pCommand->AddRenderToTextureBarrier({ pDepthTexture.get() }, METextureBarrierStage::EPixelShaderSample);

	pCommand->BeginRenderPass(&m_peelRenderPass);


	const Vector2i f2LeftTop = info.f2ViewportLeftTop;
	const Vector2i f2Size = info.f2ViewportSize;
	pCommand->SetViewport(MViewportInfo(f2LeftTop.x, f2LeftTop.y, f2Size.x, f2Size.y));
	pCommand->SetScissor(MScissorInfo(f2LeftTop.x, f2LeftTop.y, f2Size.x, f2Size.y));


	if (m_pDrawPeelMaterial->GetTextureParams()[0]->GetTexture() != pDepthTexture)
	{
		m_pDrawPeelMaterial->GetTextureParams()[0]->SetTexture(pDepthTexture);
	}
	

	if (pCommand->SetUseMaterial(m_pDrawPeelMaterial))
	{
		pCommand->DrawMesh(pMeshManager->GetScreenRect());
	}

	for (size_t nSubpassIdx = 1; nSubpassIdx < m_peelRenderPass.m_vSubpass.size(); ++nSubpassIdx)
	{
		pCommand->NextSubPass();

		pCommand->PushShaderPropertyBlock(m_aFramePropertyBlock[nSubpassIdx % 2]);

		for (MCullingResultRenderable* pRenderable : vRenderable)
		{
			pRenderable->Render(pCommand);
		}

		pCommand->PopShaderPropertyBlock();
	}

	pCommand->EndRenderPass();
}

void MTransparentRenderWork::RenderScreenFill(MRenderInfo& info)
{
	MIRenderCommand* pCommand = info.pPrimaryRenderCommand;
	if (!pCommand)
	{
		MORTY_ASSERT(pCommand);
		return;
	}

	MMeshManager* pMeshManager = GetEngine()->FindGlobalObject<MMeshManager>();
	if (!pMeshManager)
	{
		MORTY_ASSERT(pMeshManager);
		return;
	}

	pCommand->AddRenderToTextureBarrier({ m_pFrontTexture.get(), m_pBackTexture.get() }, METextureBarrierStage::EPixelShaderSample);

	pCommand->BeginRenderPass(&m_renderPass);

	const Vector2i f2LeftTop = info.f2ViewportLeftTop;
	const Vector2i f2Size = info.f2ViewportSize;
	pCommand->SetViewport(MViewportInfo(f2LeftTop.x, f2LeftTop.y, f2Size.x, f2Size.y));
	pCommand->SetScissor(MScissorInfo(0.0f, 0.0f, f2Size.x, f2Size.y));

	pCommand->SetUseMaterial(m_pDrawFillMaterial);

	pCommand->DrawMesh(pMeshManager->GetScreenRect());

	pCommand->EndRenderPass();
}

void MTransparentRenderWork::InitializeMaterial()
{
	MResourceSystem* pResourceSystem = GetEngine()->FindSystem<MResourceSystem>();

	std::shared_ptr<MResource> pDPVSResource = pResourceSystem->LoadResource("Shader/Forward/depth_peel_blend.mvs");
	std::shared_ptr<MResource> pDPBPSResource = pResourceSystem->LoadResource("Shader/Forward/depth_peel_blend.mps");
	std::shared_ptr<MResource> pDPFPSResource = pResourceSystem->LoadResource("Shader/Forward/depth_peel_fill.mps");

	m_pDrawPeelMaterial = pResourceSystem->CreateResource<MMaterialResource>();
	m_pDrawPeelMaterial->SetMaterialType(MEMaterialType::EDepthPeel);
	m_pDrawPeelMaterial->GetShaderMacro().AddUnionMacro(MRenderGlobal::MEN_TRANSPARENT, MRenderGlobal::SHADER_DEFINE_ENABLE_FLAG);
	m_pDrawPeelMaterial->LoadShader(pDPVSResource);
	m_pDrawPeelMaterial->LoadShader(pDPFPSResource);


	m_pDrawFillMaterial = pResourceSystem->CreateResource<MMaterialResource>();
	m_pDrawFillMaterial->SetMaterialType(MEMaterialType::ETransparentBlend);
	m_pDrawFillMaterial->LoadShader(pDPVSResource);
	m_pDrawFillMaterial->LoadShader(pDPBPSResource);

	std::vector<std::shared_ptr<MShaderTextureParam>>& params = m_pDrawFillMaterial->GetTextureParams();
	params[0]->SetTexture(m_pFrontTexture);
	params[1]->SetTexture(m_pBackTexture);
}

void MTransparentRenderWork::ReleaseMaterial()
{
	m_pDrawFillMaterial = nullptr;

	m_pDrawPeelMaterial = nullptr;
}

void MTransparentRenderWork::InitializeTexture()
{
	MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();
	MResourceSystem* pResourceSystem = GetEngine()->FindSystem<MResourceSystem>();

	m_pBlackTexture = pResourceSystem->CreateResource<MTextureResource>("Transparent_Black");
	m_pWhiteTexture = pResourceSystem->CreateResource<MTextureResource>("Transparent_White");

	static MByte black[4] = { 0, 0 ,0 ,0 };
	static MByte white[4] = { 255, 255 ,255 ,255 };

    m_pBlackTexture->Load(MTextureResourceUtil::LoadFromMemory("Transparent_Black", black, 1, 1, 4));
	m_pWhiteTexture->Load(MTextureResourceUtil::LoadFromMemory("Transparent_White", white, 1, 1, 4));


	Vector2i size(512, 512);


	m_pFrontTexture = MTexture::CreateRenderTarget();
	m_pFrontTexture->SetSize(size);

	m_pBackTexture = MTexture::CreateRenderTarget();
	m_pBackTexture->SetSize(size);
	
	m_pFrontDepthForPassA = MTexture::CreateRenderTargetFloat32();
	m_pFrontDepthForPassA->SetSize(size);
	m_pFrontDepthForPassA->GenerateBuffer(pRenderSystem->GetDevice());

	m_pBackDepthForPassA = MTexture::CreateRenderTargetFloat32();
	m_pBackDepthForPassA->SetSize(size);
	m_pBackDepthForPassA->GenerateBuffer(pRenderSystem->GetDevice());

	m_pFrontDepthForPassB = MTexture::CreateRenderTargetFloat32();
	m_pFrontDepthForPassB->SetSize(size);
	m_pFrontDepthForPassB->GenerateBuffer(pRenderSystem->GetDevice());

	m_pBackDepthForPassB = MTexture::CreateRenderTargetFloat32();
	m_pBackDepthForPassB->SetSize(size);
	m_pBackDepthForPassB->GenerateBuffer(pRenderSystem->GetDevice());

}

void MTransparentRenderWork::ReleaseTexture()
{
	const MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();

	m_pBlackTexture->GetTextureTemplate()->DestroyBuffer(pRenderSystem->GetDevice());
	m_pWhiteTexture->GetTextureTemplate()->DestroyBuffer(pRenderSystem->GetDevice());

	m_pBlackTexture = nullptr;
	m_pWhiteTexture = nullptr;

	if (m_pFrontTexture)
	{
		m_pFrontTexture->DestroyBuffer(pRenderSystem->GetDevice());
		m_pFrontTexture = nullptr;
	}

	if (m_pBackTexture)
	{
		m_pBackTexture->DestroyBuffer(pRenderSystem->GetDevice());
		m_pBackTexture = nullptr;
	}

	if (m_pFrontDepthForPassA)
	{
		m_pFrontDepthForPassA->DestroyBuffer(pRenderSystem->GetDevice());
		m_pFrontDepthForPassA = nullptr;
	}

	if (m_pBackDepthForPassA)
	{
		m_pBackDepthForPassA->DestroyBuffer(pRenderSystem->GetDevice());
		m_pBackDepthForPassA = nullptr;
	}

	if (m_pFrontDepthForPassB)
	{
		m_pFrontDepthForPassB->DestroyBuffer(pRenderSystem->GetDevice());
		m_pFrontDepthForPassB = nullptr;
	}

	if (m_pBackDepthForPassB)
	{
		m_pBackDepthForPassB->DestroyBuffer(pRenderSystem->GetDevice());
		m_pBackDepthForPassB = nullptr;
	}
}

void MTransparentRenderWork::InitializePeelRenderPass()
{
#if MORTY_DEBUG
	m_peelRenderPass.m_strDebugName = "Transparent Peel";
#endif

	m_peelRenderPass.AddBackTexture(m_pFrontTexture, { true, MColor::Black_T });
	
	m_peelRenderPass.AddBackTexture(m_pBackTexture, { true, MColor::Black_T });
	
	m_peelRenderPass.AddBackTexture(m_pFrontDepthForPassA, { true, MColor::White });
	
	m_peelRenderPass.AddBackTexture(m_pBackDepthForPassA, { true, MColor::Black_T });
	
	m_peelRenderPass.AddBackTexture(m_pFrontDepthForPassB, { true, MColor::White });
	
	m_peelRenderPass.AddBackTexture(m_pBackDepthForPassB, { true, MColor::Black_T });


	static constexpr uint32_t SUB_PASS_NUM = 6;

	m_peelRenderPass.m_vSubpass.push_back(MSubpass());
	MSubpass& subpass = m_peelRenderPass.m_vSubpass.back();

	/*
	* 0 output front
	* 1 output back
	* 2 input/output front
	* 3 input/output back
	* 4 input/output front depth
	* 5 input/output back depth
	*/

	subpass.m_vInputIndex = {};
	subpass.m_vOutputIndex = { 0, 1, 2, 3 };

	for (uint32_t i = 0; i < SUB_PASS_NUM; ++i)
	{
		m_peelRenderPass.m_vSubpass.push_back(MSubpass());
		MSubpass& subpass = m_peelRenderPass.m_vSubpass.back();

		if (i % 2)
		{
			subpass.m_vInputIndex = { 4, 5 };
			subpass.m_vOutputIndex = { 0, 1, 2, 3 };
		}
		else
		{
			subpass.m_vInputIndex = { 2, 3 };
			subpass.m_vOutputIndex = { 0, 1, 4, 5 };
		}
	}

	m_peelRenderPass.SetDepthTestEnable(true);
	m_peelRenderPass.SetDepthWriteEnable(false);
}

void MTransparentRenderWork::ReleasePeelRenderPass()
{
	MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();

	m_peelRenderPass.DestroyBuffer(pRenderSystem->GetDevice());
}

void MTransparentRenderWork::InitializeFillRenderPass()
{
#if MORTY_DEBUG
	m_renderPass.m_strDebugName = "Transparent Fill";
#endif

	m_renderPass.SetDepthTestEnable(false);
	m_renderPass.SetDepthWriteEnable(false);
}

void MTransparentRenderWork::ReleaseFillRenderPass()
{
	MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();

	m_renderPass.DestroyBuffer(pRenderSystem->GetDevice());
}

void MTransparentRenderWork::InitializeFrameShaderParams()
{
	MResourceSystem* pResourceSystem = GetEngine()->FindSystem<MResourceSystem>();

	std::shared_ptr<MResource> forwardVS = pResourceSystem->LoadResource("Shader/Model/universal_model.mvs");
	std::shared_ptr<MResource> forwardPS = pResourceSystem->LoadResource("Shader/Forward/basic_lighting.mps");
	m_pForwardMaterial = pResourceSystem->CreateResource<MMaterialResource>();
	m_pForwardMaterial->SetCullMode(MECullMode::ECullBack);
	m_pForwardMaterial->SetMaterialType(MEMaterialType::EDepthPeel);
	m_pForwardMaterial->GetShaderMacro().AddUnionMacro(MRenderGlobal::MEN_TRANSPARENT, MRenderGlobal::SHADER_DEFINE_ENABLE_FLAG);
	m_pForwardMaterial->LoadShader(forwardVS);
	m_pForwardMaterial->LoadShader(forwardPS);

	m_aFramePropertyBlock[0] = m_pForwardMaterial->GetShaderPropertyBlocks()[MRenderGlobal::SHADER_PARAM_SET_OTHER]->Clone();
	m_aFramePropertyBlock[1] = m_pForwardMaterial->GetShaderPropertyBlocks()[MRenderGlobal::SHADER_PARAM_SET_OTHER]->Clone();
	m_aFramePropertyBlock[0]->SetTexture(MShaderPropertyName::TRANSPARENT_TEXTURE_INPUT_0, m_pFrontDepthForPassB);
	m_aFramePropertyBlock[0]->SetTexture(MShaderPropertyName::TRANSPARENT_TEXTURE_INPUT_1, m_pBackDepthForPassB);
	m_aFramePropertyBlock[1]->SetTexture(MShaderPropertyName::TRANSPARENT_TEXTURE_INPUT_0, m_pFrontDepthForPassA);
	m_aFramePropertyBlock[1]->SetTexture(MShaderPropertyName::TRANSPARENT_TEXTURE_INPUT_1, m_pBackDepthForPassA);
}

void MTransparentRenderWork::ReleaseFrameShaderParams()
{
	const auto pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();

	m_aFramePropertyBlock[0]->DestroyBuffer(pRenderSystem->GetDevice());
	m_aFramePropertyBlock[1]->DestroyBuffer(pRenderSystem->GetDevice());

	m_pForwardMaterial = nullptr;
}
