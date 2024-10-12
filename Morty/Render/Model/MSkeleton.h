/**
 * @File         MSkeleton
 * 
 * @Created      2019-12-03 16:27:14
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"
#include "Math/Matrix.h"
#include "Resource/MResource.h"
#include "Utility/MString.h"
#include "Variant/MVariant.h"

namespace morty
{

class MBoundsOBB;
class MShaderPropertyBlock;
class MORTY_API MBone
{
public:
    MBone();

    flatbuffers::Offset<void> Serialize(flatbuffers::FlatBufferBuilder& fbb) const;

    void                      Deserialize(const void* pBufferPointer);

public:
    MString               strName;
    uint32_t              unIndex;
    uint32_t              unParentIndex;
    std::vector<uint32_t> vChildrenIndices;

    //Bones World
    Matrix4               m_matTransform;
    Matrix4               m_matOffsetMatrix;
};

struct MORTY_API MSkeletonPose {
    std::vector<Matrix4> vBoneMatrix;
};

class MORTY_API MSkeleton : public MTypeClass
{
public:
    MORTY_CLASS(MSkeleton)
public:
    MSkeleton() = default;

    virtual ~MSkeleton() = default;

    const std::map<MString, uint32_t>& GetBonesMap() const { return m_bonesMap; }

    void                               CopyAllBones(std::vector<MBone*>& allBones);

    MBone*                             FindBoneByName(const MString& strName);

    const MBone*                       FindBoneByName(const MString& strName) const;

    MBone*                             AppendBone(const MString& strName);

    void                               SortByDeep();

    void                               RebuildBonesMap();

    const std::vector<MBone>&          GetAllBones() const { return m_allBones; }

    flatbuffers::Offset<void>          Serialize(flatbuffers::FlatBufferBuilder& fbb) const;

    void                               Deserialize(const void* pBufferPointer);

private:
    friend class MSkeletonResource;

private:
    std::map<MString, uint32_t> m_bonesMap;
    std::vector<MBone>          m_allBones;
};

}// namespace morty