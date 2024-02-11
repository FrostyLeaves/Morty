#include "System/MSkyBoxSystem.h"

#include "Engine/MEngine.h"
#include "Render/MIDevice.h"
#include "Render/MRenderCommand.h"

#include "System/MObjectSystem.h"
#include "System/MRenderSystem.h"

#include "Component/MSkyBoxComponent.h"
#include "RenderProgram/RenderWork/MEnvironmentMapRenderWork.h"
#include "TaskGraph/MTaskGraph.h"
#include "Utility/MFunction.h"

using namespace morty;

MORTY_CLASS_IMPLEMENT(MSkyBoxSystem, MISystem)


MSkyBoxSystem::MSkyBoxSystem()
	: MISystem()
{
}

MSkyBoxSystem::~MSkyBoxSystem()
{
}

void MSkyBoxSystem::GenerateEnvironmentWork(MSkyBoxComponent* pSkyBoxComponent)
{
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
		}

    	pRenderWork->DeleteLater();
	});

	pCommand->RenderCommandEnd();

	pDevice->SubmitCommand(pCommand);
}

void MSkyBoxSystem::GenerateEnvironmentTexture(MSkyBoxComponent* pComponent)
{
	MThreadPool* pThreadPool = GetEngine()->GetThreadPool();

	MThreadWork work(METhreadType::ERenderThread);
	work.funcWorkFunction = M_CLASS_FUNCTION_BIND_1_0(MSkyBoxSystem::GenerateEnvironmentWork, this, pComponent);
	pThreadPool->AddWork(work);
}
