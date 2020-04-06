/**
 * @File         MModelMeshStruct
 * 
 * @Created      2020-03-21 18:22:36
 *
 * @Author       Pobrecito
**/

#ifndef _M_MODELMESHSTRUCT_H_
#define _M_MODELMESHSTRUCT_H_
#include "MGlobal.h"
#include "MString.h"
#include "MMesh.h"

#include <map>

class MIMesh;
class MBoundsOBB;
class MBoundsSphere;
class MSkeleton;
class MMultiLevelMesh;
class MModelResource;
class MORTY_CLASS MModelMeshStruct
{
public:
	enum MEMeshVertexType {
		Normal = 0,
		Skeleton = 1,
	};
public:
	MModelMeshStruct();
    virtual ~MModelMeshStruct();

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
	MBoundsSphere* m_pBoundsSphere;
	MMaterial* m_pMaterial;
	MModelResource* m_pResource;

	MMultiLevelMesh* m_pMeshDetailMap;
};

#endif
