#include "MRenderSystem.h"


#include "MEngine.h"
#include "MVulkanDevice.h"
#include "MVulkanRenderCommand.h"

MORTY_CLASS_IMPLEMENT(MRenderSystem, MISystem)

MRenderSystem::MRenderSystem()
	: MISystem()
	, m_pDevice(nullptr)
	, m_unFrameCount(0)
	, m_vWaitRenderCommand()
{

}

MRenderSystem::~MRenderSystem()
{

}

void MRenderSystem::Update(MTaskNode* pNode)
{
	CheckFrameFinish();
}

MIRenderCommand* MRenderSystem::NextFrame()
{
	MIRenderCommand* pCommand = m_pDevice->CreateRenderCommand();

	if (MVulkanRenderCommand* pVulkanCommand = static_cast<MVulkanRenderCommand*>(pCommand))
	{
		pVulkanCommand->m_unFrameIndex = ++m_unFrameCount;
	}

	m_vWaitRenderCommand.push_back(pCommand);

	m_pDevice->NewFrame(m_unFrameCount);

	GetEngine()->GetLogger()->Information("the Frame Begin: %d", m_unFrameCount);

	return pCommand;
}

MIDevice* MRenderSystem::GetDevice()
{
	return m_pDevice;
}

void MRenderSystem::CheckFrameFinish()
{
	for (auto iter = m_vWaitRenderCommand.begin(); iter != m_vWaitRenderCommand.end();)
	{
		MIRenderCommand* pCommand = *iter;
		if (m_pDevice->IsFinishedCommand(pCommand))
		{
			uint32_t unFrameIdx = pCommand->GetFrameIndex();
			GetEngine()->GetLogger()->Information("the Frame Finished: %d", unFrameIdx);
			m_pDevice->RecoveryRenderCommand(pCommand);
			m_pDevice->FrameFinish(unFrameIdx);

			iter = m_vWaitRenderCommand.erase(iter);
		}
		else
		{
			++iter;
		}
	}
}

void MRenderSystem::WiteAllFrameFinish()
{
	while (!m_vWaitRenderCommand.empty())
	{
		CheckFrameFinish();
	}
}

void MRenderSystem::Initialize()
{
	if (!m_pDevice)
	{
		m_pDevice = new MVulkanDevice();
		m_pDevice->SetEngine(GetEngine());
		m_pDevice->Initialize();
	}

}

void MRenderSystem::Release()
{
	WiteAllFrameFinish();

	if (m_pDevice)
	{
		m_pDevice->Release();
		delete m_pDevice;
		m_pDevice = nullptr;
	}
}
