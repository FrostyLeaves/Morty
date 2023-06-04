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
#include "Utility/MGlobal.h"
#include "Utility/MString.h"
#include "Render/MMesh.h"
#include "Resource/MResource.h"
#include "Utility/MBounds.h"
#include "Resource/MResourceLoader.h"

#include <map>

class MIMesh;
class MSkeleton;
class MMultiLevelMesh;


enum class MEMeshVertexType
{
	Normal = 0,
	Skeleton,
};

struct MORTY_API MMeshResourceData : public MFbResourceData
{
public:
	//RawData
	MEMeshVertexType eVertexType = MEMeshVertexType::Normal;
	MIMesh* pMesh = nullptr;
	MBoundsOBB boundsOBB;
	MBoundsSphere boundsSphere;
	
	flatbuffers::Offset<void> Serialize(flatbuffers::FlatBufferBuilder& fbb) const override;
	void Deserialize(const void* pBufferPointer) override;

};

class MORTY_API MMeshResource : public MResource
{
public:
	MORTY_CLASS(MMeshResource);

public:
	MMeshResource();
    virtual ~MMeshResource();

	MEMeshVertexType GetMeshVertexType() const;
	MIMesh* GetMesh() { return m_pMesh; }
	MIMesh* GetLevelMesh(const uint32_t unLevel);
	const MBoundsOBB* GetMeshesDefaultOBB() const;
	const MBoundsSphere* GetMeshesDefaultSphere() const;


public:

	bool Load(std::unique_ptr<MResourceData>& pResourceData) override;
	virtual bool SaveTo(std::unique_ptr<MResourceData>& pResourceData) override;

	virtual void OnDelete() override;

public:

	static MIMesh* CreateMeshFromType(const MEMeshVertexType& eType);


protected:

	void Clean();
	
	void ResetBounds();

private:

	friend class MEngine;
	friend class MModelConverter;

	MIMesh* m_pMesh = nullptr;
	std::unique_ptr<MResourceData> m_pResourceData = nullptr;

	MMultiLevelMesh* m_pMeshDetailMap = nullptr;
};

class MORTY_API MMeshResourceLoader : public MResourceLoaderTemplate<MMeshResource, MMeshResourceData>
{
public:
	static MString GetResourceTypeName() { return "Mesh"; }
	static std::vector<MString> GetSuffixList() { return { "mesh" }; }
};
#endif
