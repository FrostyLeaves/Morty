/**
 * @File         MModelMeshData
 * 
 * @Created      2020-03-21 18:22:36
 *
 * @Author       Pobrecito
**/

#ifndef _M_MODELMESHDATA_H_
#define _M_MODELMESHDATA_H_
#include "MGlobal.h"
#include "MString.h"
#include "MMesh.h"

#include <map>

class MIMesh;
class MBoundsOBB;
class MSkeleton;
class MMeshDetailMap;
class MModelResource;
class MORTY_CLASS MModelMeshData
{
public:
	enum MEMeshVertexType {
		Normal = 0,
		Skeleton = 1,
	};
public:
	MModelMeshData();
    virtual ~MModelMeshData();

	MString GetMeshName() { return m_strName; }
	MEMeshVertexType GetMeshVertexType(){ return m_eVertexType; }
	void SetMesh(MIMesh* pMesh) { m_pMesh = pMesh; }
	MIMesh* GetMesh() { return m_pMesh; }
	MIMesh* GetLevelMesh(const unsigned int unLevel);
	const MBoundsOBB* GetMeshesDefaultOBB() { return m_pBoundsOBB; }
	const Matrix4* GetMeshesRotationMatrix() { return &m_matRotationMatrix; }
	MMaterial* GetDefaultMaterial() { return m_pMaterial; }
	MModelResource* GetModelResource() { return m_pResource; }

protected:

private:

	friend class MModelResource;
    
	MString m_strName;
	MEMeshVertexType m_eVertexType;
	MIMesh* m_pMesh;
	Matrix4 m_matRotationMatrix;
	MBoundsOBB* m_pBoundsOBB;
	MMaterial* m_pMaterial;
	MModelResource* m_pResource;

	MMeshDetailMap* m_pMeshDetailMap;
};

#endif
