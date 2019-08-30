#include "MSpatial.h"
#include "MModelResource.h"
#include "MModel.h"
#include "MMeshInstance.h"

MSpatial::MSpatial()
{

}

MSpatial::~MSpatial()
{

}

bool MSpatial::Load(MResource* pResource)
{
	if (MModelResource* pModelRes = dynamic_cast<MModelResource*>(pResource))
	{
		m_pResource = pResource;

		MModel* pModel = pModelRes->GetModelTemplate();

		for (MMesh* pMesh : pModel->GetMeshes())
		{
			MMeshInstance* pMeshIns = GetObjectManager()->CreateObject<MMeshInstance>();
			pMeshIns->SetMesh(pMesh);
			AddNode(pMeshIns);
		}


		//Do smoething.
		return true;
	}

	return false;
}
