#include "System/MSkyBoxSystem.h"

#include "Engine/MEngine.h"
#include "Render/MIDevice.h"
#include "Render/MRenderCommand.h"

#include "System/MObjectSystem.h"
#include "System/MRenderSystem.h"

#include "Component/MSkyBoxComponent.h"
#include "RenderProgram/MEnvironmentMapRenderWork.h"

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

	if (!pSkyBoxComponent)
		return;

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
