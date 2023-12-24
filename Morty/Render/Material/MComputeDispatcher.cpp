#include "MComputeDispatcher.h"
#include "Shader/MShader.h"
#include "Resource/MShaderResource.h"
#include "Resource/MMaterialResource.h"
#include "Engine/MEngine.h"
#include "Render/MIDevice.h"

#include "System/MRenderSystem.h"
#include "System/MResourceSystem.h"


MORTY_CLASS_IMPLEMENT(MComputeDispatcher, MObject)

bool MComputeDispatcher::LoadComputeShader(std::shared_ptr<MResource> pResource)
{
	bool bResult = m_pShaderProgram->LoadShader(pResource);
	
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
	return m_pShaderProgram->GetShader(MEShaderType::ECompute);
}

void MComputeDispatcher::OnCreated()
{
	Super::OnCreated();

	MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();
	pRenderSystem->GetDevice()->RegisterComputeDispatcher(this);

	m_pShaderProgram = MShaderProgram::MakeShared(GetEngine(), MShaderProgram::EUsage::ECompute);
}

void MComputeDispatcher::OnDelete()
{
	MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();
	pRenderSystem->GetDevice()->UnRegisterComputeDispatcher(this);

	m_pShaderProgram->ClearShader();
		
	Super::OnDelete();
}
