#include "Model/MSkeleton.h"
#include <algorithm>

#include "Batch/BatchGroup/MInstanceBatchGroup.h"
#include "Engine/MEngine.h"
#include "Flatbuffer/MBone_generated.h"
#include "Flatbuffer/MSkeleton_generated.h"
#include "Shader/MShaderPropertyBlock.h"
#include "System/MRenderSystem.h"
#include "Utility/MFileHelper.h"

using namespace morty;

MORTY_CLASS_IMPLEMENT(MSkeleton, MTypeClass)

MBone::MBone()
    : m_matTransform(Matrix4::IdentityMatrix)
    , m_matOffsetMatrix(Matrix4::IdentityMatrix)
{}

flatbuffers::Offset<void> MBone::Serialize(flatbuffers::FlatBufferBuilder& fbb) const
{
    auto              fbName      = fbb.CreateString(strName);
    auto              fbTransform = m_matTransform.Serialize(fbb);
    auto              fbOffset    = m_matOffsetMatrix.Serialize(fbb);
    auto              fbChildren  = fbb.CreateVector(vChildrenIndices);

    fbs::MBoneBuilder builder(fbb);

    builder.add_name(fbName);
    builder.add_index(unIndex);
    builder.add_parent(unParentIndex);
    builder.add_transform(fbTransform);
    builder.add_offset(fbOffset);
    builder.add_children(fbChildren);

    return builder.Finish().Union();
}

void MBone::Deserialize(const void* pBufferPointer)
{
    const fbs::MBone* fbData = reinterpret_cast<const fbs::MBone*>(pBufferPointer);

    strName       = fbData->name()->c_str();
    unIndex       = fbData->index();
    unParentIndex = fbData->parent();
    m_matTransform.Deserialize(fbData->transform());
    m_matOffsetMatrix.Deserialize(fbData->offset());

    vChildrenIndices.resize(fbData->children()->size());
    for (size_t idx = 0; idx < fbData->children()->size(); ++idx)
    {
        vChildrenIndices[idx] = fbData->children()->Get(static_cast<uint32_t>(idx));
    }
}

void MSkeleton::CopyAllBones(std::vector<MBone*>& allBones)
{
    for (MBone* pBone: allBones) delete pBone;

    allBones.resize(m_allBones.size());

    for (size_t i = 0; i < m_allBones.size(); ++i)
    {
        allBones[i]    = new MBone();
        (*allBones[i]) = (m_allBones[i]);
    }
}

MBone* MSkeleton::FindBoneByName(const MString& strName)
{
    auto iter = m_bonesMap.find(strName);
    if (iter != m_bonesMap.cend()) { return &m_allBones[iter->second]; }

    return nullptr;
}

const MBone* MSkeleton::FindBoneByName(const MString& strName) const
{
    auto iter = m_bonesMap.find(strName);
    if (iter != m_bonesMap.cend()) { return &m_allBones[iter->second]; }

    return nullptr;
}

MBone* MSkeleton::AppendBone(const MString& strName)
{
    m_allBones.push_back(MBone());
    MBone& bone         = m_allBones.back();
    bone.strName        = strName;
    bone.unIndex        = static_cast<uint32_t>(m_allBones.size()) - 1;
    bone.unParentIndex  = MGlobal::M_INVALID_INDEX;
    m_bonesMap[strName] = bone.unIndex;
    return &m_allBones.back();
}

void MSkeleton::SortByDeep()
{
    std::vector<int>        map(m_allBones.size());
    std::map<uint32_t, int> tDeep;

    for (MBone& bone: m_allBones)
    {
        int      deep        = 0;
        uint32_t unParentIdx = bone.unIndex;
        while (unParentIdx != MGlobal::M_INVALID_UINDEX)
        {
            unParentIdx = m_allBones[unParentIdx].unParentIndex;
            ++deep;
        }

        tDeep[bone.unIndex] = deep;
    }

    std::vector<MBone>& vBones = m_allBones;

    std::sort(vBones.begin(), vBones.end(), [&tDeep](MBone& a, MBone& b) {
        return tDeep[a.unIndex] < tDeep[b.unIndex];
    });

    for (uint32_t i = 0; i < vBones.size(); ++i) map[vBones[i].unIndex] = i;

    for (uint32_t i = 0; i < vBones.size(); ++i)
    {
        MBone& bone  = vBones[i];
        bone.unIndex = map[bone.unIndex];
        if (MGlobal::M_INVALID_UINDEX != bone.unParentIndex) bone.unParentIndex = map[bone.unParentIndex];
        for (uint32_t& index: bone.vChildrenIndices) index = map[index];
    }

    for (auto& iter: m_bonesMap) iter.second = map[iter.second];
}

void MSkeleton::RebuildBonesMap()
{
    m_bonesMap.clear();
    for (MBone& bone: m_allBones) { m_bonesMap[bone.strName] = bone.unIndex; }
}

flatbuffers::Offset<void> MSkeleton::Serialize(flatbuffers::FlatBufferBuilder& fbb) const
{
    std::vector<flatbuffers::Offset<MBone>> vBonesOffset(m_allBones.size());

    for (size_t nIdx = 0; nIdx < m_allBones.size(); ++nIdx) { vBonesOffset[nIdx] = m_allBones[nIdx].Serialize(fbb).o; }

    auto                  fbBones = fbb.CreateVector(vBonesOffset).o;

    fbs::MSkeletonBuilder builder(fbb);

    builder.add_bones(fbBones);

    return builder.Finish().Union();
}

void MSkeleton::Deserialize(const void* pBufferPointer)
{
    const fbs::MSkeleton* fbData = reinterpret_cast<const fbs::MSkeleton*>(pBufferPointer);

    m_allBones.resize(fbData->bones()->size());
    for (size_t idx = 0; idx < fbData->bones()->size(); ++idx)
    {
        m_allBones[idx].Deserialize(fbData->bones()->Get(static_cast<uint32_t>(idx)));
    }
}
