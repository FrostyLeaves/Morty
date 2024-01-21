#include "MSinglePassRenderWork.h"

#include "MVRSTextureRenderWork.h"
#include "Scene/MScene.h"
#include "Engine/MEngine.h"
#include "Render/MIDevice.h"
#include "Basic/MTexture.h"
#include "Basic/MViewport.h"
#include "Model/MSkeleton.h"
#include "Material/MMaterial.h"
#include "Render/MRenderPass.h"
#include "Render/MRenderCommand.h"

#include "System/MRenderSystem.h"
#include "System/MResourceSystem.h"

#include "Component/MSceneComponent.h"
#include "Component/MCameraComponent.h"
#include "Component/MRenderMeshComponent.h"
#include "Component/MDirectionalLightComponent.h"
#include "Render/MVertex.h"

#include "Utility/MBounds.h"
#include "Mesh/MMeshManager.h"

MORTY_INTERFACE_IMPLEMENT(ISinglePassRenderWork, MRenderTaskNode)

void ISinglePassRenderWork::Initialize(MEngine* pEngine)
{
	m_pEngine = pEngine;

#if MORTY_DEBUG
	m_renderPass.m_strDebugName = GetTypeName();
#endif
}

void ISinglePassRenderWork::Release()
{
	MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();
	m_renderPass.DestroyBuffer(pRenderSystem->GetDevice());
}

std::vector<std::shared_ptr<MTexture>> ISinglePassRenderWork::GetBackTextures() const
{
	return m_renderPass.GetBackTextures();
}

std::shared_ptr<MTexture> ISinglePassRenderWork::GetDepthTexture() const
{
	return m_renderPass.GetDepthTexture();
}

std::shared_ptr<IGetTextureAdapter> ISinglePassRenderWork::CreateOutput() const
{
	auto pOutput = std::make_shared<MGetTextureAdapter>(m_renderPass.GetBackTexture(0));

	return pOutput;
}

MRenderTargetGroup ISinglePassRenderWork::AutoBindTarget()
{
	MRenderTargetGroup group;

	auto vOutputDesc = GetOutputName();
	MORTY_ASSERT(vOutputDesc.size() == GetOutputSize());

	for (size_t nIdx = 0; nIdx < GetOutputSize(); ++nIdx)
	{
		auto pTexture = GetRenderOutput(nIdx)->GetTexture();
		
		if(pTexture->GetRenderUsage() == METextureWriteUsage::ERenderBack || pTexture->GetRenderUsage() == METextureWriteUsage::ERenderPresent)
		{
			group.backTargets.push_back({ pTexture, vOutputDesc[nIdx].renderDesc });
		}
		else if (pTexture->GetRenderUsage() == METextureWriteUsage::ERenderDepth)
		{
			MORTY_ASSERT(group.depthTarget.pTexture == nullptr);
			group.depthTarget = { pTexture, vOutputDesc[nIdx].renderDesc };
		}
        else
        {
			MORTY_ASSERT(false);
        }
	}

	return group;
}

MRenderTargetGroup ISinglePassRenderWork::AutoBindTargetWithVRS()
{
	auto group = AutoBindTarget();

	auto pVRSTexture = GetRenderTargetManager()->FindRenderTexture(MVRSTextureRenderWork::VRS_TEXTURE);
	group.shadingRate = { pVRSTexture, {false, MColor::Black_T} };

	return group;
}

void ISinglePassRenderWork::AutoBindBarrierTexture()
{
	std::set<MTexture*> tInputTextures;
	for (size_t nInputIdx = 0; nInputIdx < GetInputSize(); ++nInputIdx)
	{
		if (auto pInputTexture = GetInputTexture(nInputIdx))
		{
			tInputTextures.insert(pInputTexture.get());
		}
	}

	for (size_t nOutputIdx = 0; nOutputIdx < GetOutputSize(); ++nOutputIdx)
	{
		if (auto pOutputTexture = GetRenderOutput(nOutputIdx)->GetTexture())
		{
			tInputTextures.erase(pOutputTexture.get());
		}
	}

	m_vBarrierTexture = { tInputTextures.begin(), tInputTextures.end() };
}

void ISinglePassRenderWork::Resize(Vector2i size)
{
	MRenderSystem* pRenderSystem = m_pEngine->FindSystem<MRenderSystem>();

	if (m_renderPass.GetFrameBufferSize() != size)
	{
		m_renderPass.Resize(pRenderSystem->GetDevice());

		//MRenderSystem* pRenderSystem = m_pEngine->FindSystem<MRenderSystem>();
		//pRenderSystem->ResizeFrameBuffer(m_renderPass, size);
	}
}

void ISinglePassRenderWork::SetRenderTarget(const MRenderTargetGroup& renderTarget)
{
	m_renderPass.SetRenderTarget(renderTarget);

	MRenderSystem* pRenderSystem = m_pEngine->FindSystem<MRenderSystem>();
	m_renderPass.DestroyBuffer(pRenderSystem->GetDevice());
	m_renderPass.GenerateBuffer(pRenderSystem->GetDevice());
}
