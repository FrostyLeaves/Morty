#include "MMeshInstance.h"
#include "MModelResource.h"

#include "MDirectX11Renderer.h"
#include "MModelResource.h"
#include "MModel.h"

MMeshInstance::MMeshInstance()
	: M3DNode()
{

}

MMeshInstance::~MMeshInstance()
{

}

bool MMeshInstance::Load(MResource* pResource)
{
	if (m_pResource = dynamic_cast<MModelResource*>(pResource))
	{
		//Do smoething.
		return true;
	}

	return false;
}

void MMeshInstance::OnTick(const float& fDelta)
{

}

void MMeshInstance::Render()
{
	if (m_pResource == nullptr)
		return;

	const std::vector<MMesh*>& meshes = m_pResource->GetModelTemplate()->GetMeshes();

	for (MMesh* pMesh : meshes)
	{
	}

}
