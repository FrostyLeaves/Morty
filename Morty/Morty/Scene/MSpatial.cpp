#include "MSpatial.h"
#include "MModelResource.h"
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

		for (MIMesh* pMesh : pModelRes->GetMeshes())
		{
			MMeshInstance* pMeshIns = GetObjectManager()->CreateObject<MMeshInstance>();
			pMeshIns->SetMesh(pMesh);
			AddNode(pMeshIns);
		}


		//Do something.
		return true;
	}

	return false;
}
