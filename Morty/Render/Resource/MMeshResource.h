/**
 * @File         MModelMeshStruct
 * 
 * @Created      2020-03-21 18:22:36
 *
 * @Author       DoubleYe
 *
 * MModelMeshStruct --> MMeshResource	2020-06-13 13:18:36
 * 
**/

#pragma once

#include "Utility/MGlobal.h"
#include "Mesh/MMesh.h"
#include "Mesh/MMeshUtil.h"
#include "Resource/MResource.h"
#include "Resource/MResourceLoader.h"
#include "Utility/MBounds.h"
#include "Utility/MString.h"

namespace morty
{

class MIMesh;
class MSkeleton;
class MMultiLevelMesh;
struct MORTY_API MMeshResourceData : public MFbResourceData {
public:
    //RawData
    MEMeshVertexType          eVertexType = MEMeshVertexType::Normal;
    std::shared_ptr<MIMesh>   pMesh       = nullptr;
    MBoundsOBB                boundsOBB;
    MBoundsSphere             boundsSphere;

    flatbuffers::Offset<void> Serialize(flatbuffers::FlatBufferBuilder& fbb) const override;

    void                      Deserialize(const void* pBufferPointer) override;
};

class MORTY_API MMeshResource : public MResource
{
public:
    MORTY_CLASS(MMeshResource);

public:
    MMeshResource();

    virtual ~MMeshResource();

    MEMeshVertexType     GetMeshVertexType() const;

    MIMesh*              GetMesh() const;

    MIMesh*              GetLevelMesh(const uint32_t unLevel);

    const MBoundsOBB*    GetMeshesDefaultOBB() const;

    const MBoundsSphere* GetMeshesDefaultSphere() const;


public:
    bool         Load(std::unique_ptr<MResourceData>&& pResourceData) override;

    virtual bool SaveTo(std::unique_ptr<MResourceData>& pResourceData) override;

    virtual void OnDelete() override;

public:
protected:
    void Clean();

    void ResetBounds();

private:
    friend class MEngine;

    friend class MModelConverter;

    std::unique_ptr<MResourceData> m_resourceData = nullptr;

    MMultiLevelMesh*               m_meshDetailMap = nullptr;
};

class MORTY_API MMeshResourceLoader : public MResourceLoaderTemplate<MMeshResource, MMeshResourceData>
{
public:
    static MString              GetResourceTypeName() { return "Mesh"; }

    static std::vector<MString> GetSuffixList() { return {"mesh"}; }
};

}// namespace morty