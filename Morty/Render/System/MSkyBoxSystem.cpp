#include "MSkyBoxSystem.h"

#include "MEngine.h"
#include "MIDevice.h"
#include "MRenderCommand.h"

#include "MObjectSystem.h"
#include "MRenderSystem.h"

#include "MSkyBoxComponent.h"
#include "MEnvironmentMapRenderWork.h"

MORTY_CLASS_IMPLEMENT(MSkyBoxSystem, MISystem)


MSkyBoxSystem::MSkyBoxSystem()
	: MISystem()
{
}

MSkyBoxSystem::~MSkyBoxSystem()
{
}

void MSkyBoxSystem::EngineTick(const float& fDelta)
{
	MSkyBoxComponent* pSkyBoxComponent = nullptr;

	while (!pSkyBoxComponent && !m_vGenerateQueue.empty())
	{
		pSkyBoxComponent = m_vGenerateQueue.front();
		m_vGenerateQueue.pop();
	}


	MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();
	if (!pRenderSystem)
		return;

	MObjectSystem* pObjectSystem = GetEngine()->FindSystem<MObjectSystem>();
	if (!pObjectSystem)
		return;

	MEnvironmentMapRenderWork* pRenderWork = pObjectSystem->CreateObject<MEnvironmentMapRenderWork>();

	MIDevice* pDevice = pRenderSystem->GetDevice();

	MIRenderCommand* pCommand = pDevice->CreateRenderCommand("Render Environment");

	pCommand->RenderCommandBegin();

	pRenderWork->RenderEnvironment(pCommand, pSkyBoxComponent);

	//TODO warning. component maybe null.
	pCommand->addFinishedCallback([=]() {
		if (pSkyBoxComponent)
		{
			pSkyBoxComponent->LoadDiffuseEnvResource(pRenderWork->GetDiffuseOutputTexture());
			pRenderWork->DeleteLater();
		}
	});

	pCommand->RenderCommandEnd();

	pDevice->SubmitCommand(pCommand);
}

void MSkyBoxSystem::GenerateEnvironmentTexture(MSkyBoxComponent* pComponent)
{
	m_vGenerateQueue.push(pComponent);
}
