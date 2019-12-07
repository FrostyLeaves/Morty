#include "MModelInstance.h"
#include "MModelResource.h"
#include "MMeshInstance.h"

MModelInstance::MModelInstance()
{

}

MModelInstance::~MModelInstance()
{

}

bool MModelInstance::Load(MResource* pResource)
{
	if (MModelResource* pModelRes = dynamic_cast<MModelResource*>(pResource))
	{
		m_pResource = pModelRes;

		int index = 0;
		char svIndexx[16];
		for (MIMesh* pMesh : *pModelRes->GetMeshes())
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
