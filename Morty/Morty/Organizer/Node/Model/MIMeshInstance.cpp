#include "MIMeshInstance.h"
#include "MEngine.h"
#include "MResourceManager.h"
#include "Material/MMaterialResource.h"

M_I_OBJECT_IMPLEMENT(MIMeshInstance, M3DNode)

MIMeshInstance::MIMeshInstance()
	: M3DNode()
{

}

MIMeshInstance::~MIMeshInstance()
{

}

bool MIMeshInstance::SetMaterialPath(const MString& strPath)
{
	if (MResource* pResource = m_pEngine->GetResourceManager()->LoadResource(strPath))
	{
		if (MMaterialResource* pMaterialRes = pResource->DynamicCast<MMaterialResource>())
		{
			SetMaterial(pMaterialRes);
			return true;
		}
	}

	return false;
}

MString MIMeshInstance::GetMaterialPath()
{
	return GetMaterial() ? GetMaterial()->GetResourcePath() : "";
}
