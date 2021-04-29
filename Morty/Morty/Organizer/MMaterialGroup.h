/**
 * @File         MMaterialGroup
 * 
 * @Created      2020-05-03 11:22:55
 *
 * @Author       DoubleYe
**/

#ifndef _M_MMATERIALGROUP_H_
#define _M_MMATERIALGROUP_H_
#include "MGlobal.h"

#include <vector>

class MMaterial;
class MRenderableMeshComponent;
class MORTY_API MMaterialGroup
{
public:
    MMaterialGroup();


    bool InsertMeshComponent(MRenderableMeshComponent* pMeshComponent);
    bool RemoveMeshComponent(MRenderableMeshComponent* pMeshComponent);

    void SetDirty() { m_bDirty = true; }

public:

	MMaterial* m_pMaterial;
	std::vector<MRenderableMeshComponent*> m_vMeshComponents;


    bool m_bDirty;

};

#endif
