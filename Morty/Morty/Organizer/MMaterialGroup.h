/**
 * @File         MMaterialGroup
 * 
 * @Created      2020-05-03 11:22:55
 *
 * @Author       Pobrecito
**/

#ifndef _M_MMATERIALGROUP_H_
#define _M_MMATERIALGROUP_H_
#include "MGlobal.h"

#include <vector>

class MMaterial;
class MIMeshInstance;
class MORTY_CLASS MMaterialGroup
{
public:
    MMaterialGroup();


    bool InsertMeshInstance(MIMeshInstance* pMeshIns);
    bool RemoveMeshInstance(MIMeshInstance* pMeshIns);

    void SetDirty() { m_bDirty = true; }

public:

	MMaterial* m_pMaterial;
	std::vector<MIMeshInstance*> m_vMeshInstances;


    bool m_bDirty;

};

#endif
