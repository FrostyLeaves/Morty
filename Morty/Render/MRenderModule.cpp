#include "MRenderModule.h"
#include "MEngine.h"
#include "MFunction.h"

#include "MRenderSystem.h"
#include "MResourceSystem.h"

#include "MShaderResource.h"
#include "MTextureResource.h"
#include "MMaterialResource.h"
#include "MSkeletonResource.h"
#include "MSkeletalAnimationResource.h"

#include "MTaskGraph.h"

bool MRenderModule::Register(MEngine* pEngine)
{
	if (!pEngine)
		return false;

	if (MRenderSystem* pRenderSystem = pEngine->RegisterSystem<MRenderSystem>())
	{
		if (MTaskGraph* pTaskGraph = pEngine->GetMainGraph())
		{
			if (MTaskNode* pTaskNode = pTaskGraph->AddNode<MTaskNode>("Render_Update"))
			{
				pTaskNode->SetThreadType(METhreadType::ERenderThread);
				pTaskNode->BindTaskFunction(M_CLASS_FUNCTION_BIND_1(MRenderSystem::Update, pRenderSystem));
			}
		}
	}

	if (MResourceSystem* pResourceSystem = pEngine->FindSystem<MResourceSystem>())
	{
		pResourceSystem->RegisterResourceType<MShaderResource>();
		pResourceSystem->RegisterResourceType<MTextureResource>();
		pResourceSystem->RegisterResourceType<MMaterialResource>();
		pResourceSystem->RegisterResourceType<MSkeletonResource>();
		pResourceSystem->RegisterResourceType<MSkeletalAnimationResource>();
	}
	

	return true;
}