#include "MComputeDispatcher.h"
#include "Material/MShader.h"
#include "MShaderBuffer.h"
#include "Resource/MShaderResource.h"
#include "Resource/MTextureResource.h"
#include "Resource/MMaterialResource.h"
#include "Engine/MEngine.h"
#include "Render/MIDevice.h"
#include "Utility/MFileHelper.h"

#include "System/MRenderSystem.h"
#include "System/MResourceSystem.h"

#include "Utility/MJson.h"
#include "Utility/MVariant.h"


MORTY_CLASS_IMPLEMENT(MComputeDispatcher, MObject)

MComputeDispatcher::MComputeDispatcher()
	: MObject()
	, m_shaderGroup()
{
}

MComputeDispatcher::~MComputeDispatcher()
{
}

bool MComputeDispatcher::LoadComputeShader(std::shared_ptr<MResource> pResource)
{
	bool bResult = m_shaderGroup.LoadComputeShader(GetEngine(), pResource);
	
	return bResult;
}

bool MComputeDispatcher::LoadComputeShader(const MString& strResource)
{
	MResourceSystem* pResourceSystem = GetEngine()->FindSystem<MResourceSystem>();
	if (std::shared_ptr<MResource> pResource = pResourceSystem->LoadResource(strResource))
		return LoadComputeShader(pResource);

	return false;
}

MShader* MComputeDispatcher::GetComputeShader()
{
	return m_shaderGroup.GetComputeShader();
}

void MComputeDispatcher::OnCreated()
{
	Super::OnCreated();

	MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();
	pRenderSystem->GetDevice()->RegisterComputeDispatcher(this);
}

void MComputeDispatcher::OnDelete()
{
	MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();
	pRenderSystem->GetDevice()->UnRegisterComputeDispatcher(this);

	m_shaderGroup.ClearShader(GetEngine());
		
	Super::OnDelete();
}
