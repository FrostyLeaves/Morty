﻿/**
 * @File         MModelMeshStruct
 * 
 * @Created      2020-03-21 18:22:36
 *
 * @Author       DoubleYe
 *
 * MModelMeshStruct --> MMeshResource	2020-06-13 13:18:36
 * 
**/

#ifndef _M_MESHRESOURCE_H_
#define _M_MESHRESOURCE_H_
#include "MGlobal.h"
#include "MString.h"
#include "MMesh.h"
#include "MResource.h"
#include "MBounds.h"

#include <map>

class MIMesh;
class MSkeleton;
class MMultiLevelMesh;
class MModelResource;
class MORTY_API MMeshResource : public MResource
{
public:
	M_RESOURCE(MMeshResource);

	enum MEMeshVertexType {
		Normal = 0,
		Skeleton = 1,
	};
public:
	MMeshResource();
    virtual ~MMeshResource();

	MString GetMeshName() { return m_strName; }
	MEMeshVertexType GetMeshVertexType(){ return m_eVertexType; }
	MIMesh* GetMesh() { return m_pMesh; }
	MIMesh* GetLevelMesh(const uint32_t unLevel);
	const MBoundsOBB* GetMeshesDefaultOBB() { return &m_BoundsOBB; }
	const MBoundsSphere* GetMeshesDefaultSphere() { return &m_BoundsSphere; }
	MResource* GetDefaultMaterial() { return m_MaterialKeeper.GetResource(); }


public:

	virtual bool Load(const MString& strResourcePath) override;
	virtual bool SaveTo(const MString& strResourcePath) override;

	virtual void OnDelete() override;

protected:

	void Clean();
	
	void ResetBounds();

	MIMesh* NewMeshByType(const MEMeshVertexType& eType);

private:

	friend class MEngine;
	friend class MModelConverter;
    
	MString m_strName;
	MEMeshVertexType m_eVertexType;
	MIMesh* m_pMesh;
	MBoundsOBB m_BoundsOBB;
	MBoundsSphere m_BoundsSphere;
	MResourceKeeper m_MaterialKeeper;
	MResourceKeeper m_SkeletonKeeper;

	MMultiLevelMesh* m_pMeshDetailMap;
};

#endif
