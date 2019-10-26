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

		int index = 0;
		char svIndexx[16];
		for (MIMesh* pMesh : pModelRes->GetMeshes())
		{
			MMeshInstance* pMeshIns = GetObjectManager()->CreateObject<MMeshInstance>();
			pMeshIns->SetMesh(pMesh);
			itoa(index, svIndexx, 10);
			pMeshIns->SetName(MString("Mesh_") + svIndexx);
			AddNode(pMeshIns);
		}


		//Do something.
		return true;
	}

	return false;
}
