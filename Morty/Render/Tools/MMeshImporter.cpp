/**
 * @File         MMeshImporter
 *
 * @Created      2025-01-14
 *
 * @Author       DoubleYe
**/

#include "Tools/MMeshImporter.h"

#include "Utility/MRenderGlobal.h"
#include "Engine/MEngine.h"
#include "Mesh/MCluster.h"
#include "Model/MSkeleton.h"
#include "Resource/MMeshResource.h"
#include "System/MResourceSystem.h"
#include "Utility/MLogger.h"

#include "assimp/mesh.h"

namespace morty
{

MMeshImporter::MMeshImporter(MEngine* engine)
    : m_engine(engine)
    , m_resourceSystem(nullptr)
{
    if (m_engine) { m_resourceSystem = m_engine->GetSystem<MResourceSystem>(); }
}

MMeshImporter::~MMeshImporter() = default;

std::shared_ptr<MMeshResource> MMeshImporter::ImportMesh(aiMesh* pAiMesh, MSkeleton* pSkeleton, MString& strMeshName)
{
    if (!pAiMesh || !m_resourceSystem) { return nullptr; }
    if (m_cache[pAiMesh]) return m_cache[pAiMesh];

    strMeshName = pAiMesh->mName.C_Str();

    // Create mesh resource data
    std::unique_ptr<MResourceData> pResourceData     = std::make_unique<MMeshResourceData>();
    auto*                          pMeshResourceData = static_cast<MMeshResourceData*>(pResourceData.get());

    // Import mesh with or without skeleton
    if (pSkeleton)
    {
        // Skeletal mesh
        auto* pBonesMesh = new MMesh<MVertexWithBones>();
        ProcessMeshVertices(pAiMesh, pBonesMesh);
        ProcessMeshIndices(pAiMesh, pBonesMesh);
        BindVertexAndBones(pSkeleton, pAiMesh, pBonesMesh);

        pMeshResourceData->pMesh.reset(pBonesMesh);
        pMeshResourceData->eVertexType = MEMeshVertexType::Skeleton;
    }
    else
    {
        // Static mesh
        auto* pStaticMesh = new MMesh<MVertex>();
        ProcessMeshVertices(pAiMesh, pStaticMesh);
        ProcessMeshIndices(pAiMesh, pStaticMesh);

        pMeshResourceData->pMesh.reset(pStaticMesh);
        pMeshResourceData->eVertexType = MEMeshVertexType::Normal;
    }

    // Create mesh resource
    std::shared_ptr<MMeshResource> pMeshResource = m_resourceSystem->CreateResource<MMeshResource>();
    pMeshResource->Load(std::move(pResourceData));
    pMeshResource->ResetBounds();

    m_cache[pAiMesh] = pMeshResource;
    return pMeshResource;
}

void MMeshImporter::ProcessMeshVertices(aiMesh* pMesh, MMesh<MVertex>* pMMesh)
{
    pMMesh->CreateVertices(pMesh->mNumVertices);

    for (uint32_t i = 0; i < pMesh->mNumVertices; ++i)
    {
        MVertex& vertex = pMMesh->GetVertices()[i];

        // Position
        vertex.position.x = pMesh->mVertices[i].x;
        vertex.position.y = pMesh->mVertices[i].y;
        vertex.position.z = pMesh->mVertices[i].z;

        // Normal
        if (pMesh->mNormals)
        {
            vertex.normal.x = pMesh->mNormals[i].x;
            vertex.normal.y = pMesh->mNormals[i].y;
            vertex.normal.z = pMesh->mNormals[i].z;
        }

        // Texture coordinates
        if (pMesh->mTextureCoords[0])
        {
            vertex.texCoords.x = pMesh->mTextureCoords[0][i].x;
            vertex.texCoords.y = pMesh->mTextureCoords[0][i].y;
        }

        // Tangent
        if (pMesh->mTangents)
        {
            vertex.tangent.x = pMesh->mTangents[i].x;
            vertex.tangent.y = pMesh->mTangents[i].y;
            vertex.tangent.z = pMesh->mTangents[i].z;
        }

        // Bitangent
        if (pMesh->mBitangents)
        {
            vertex.bitangent.x = pMesh->mBitangents[i].x;
            vertex.bitangent.y = pMesh->mBitangents[i].y;
            vertex.bitangent.z = pMesh->mBitangents[i].z;
        }
    }
}

void MMeshImporter::ProcessMeshVertices(aiMesh* pMesh, MMesh<MVertexWithBones>* pMMesh)
{
    pMMesh->CreateVertices(pMesh->mNumVertices);

    for (uint32_t i = 0; i < pMesh->mNumVertices; ++i)
    {
        MVertexWithBones& vertex = pMMesh->GetVertices()[i];

        // Position
        vertex.position.x = pMesh->mVertices[i].x;
        vertex.position.y = pMesh->mVertices[i].y;
        vertex.position.z = pMesh->mVertices[i].z;

        // Normal
        if (pMesh->mNormals)
        {
            vertex.normal.x = pMesh->mNormals[i].x;
            vertex.normal.y = pMesh->mNormals[i].y;
            vertex.normal.z = pMesh->mNormals[i].z;
        }

        // Texture coordinates
        if (pMesh->mTextureCoords[0])
        {
            vertex.texCoords.x = pMesh->mTextureCoords[0][i].x;
            vertex.texCoords.y = pMesh->mTextureCoords[0][i].y;
        }

        // Tangent
        if (pMesh->mTangents)
        {
            vertex.tangent.x = pMesh->mTangents[i].x;
            vertex.tangent.y = pMesh->mTangents[i].y;
            vertex.tangent.z = pMesh->mTangents[i].z;
        }

        // Bitangent
        if (pMesh->mBitangents)
        {
            vertex.bitangent.x = pMesh->mBitangents[i].x;
            vertex.bitangent.y = pMesh->mBitangents[i].y;
            vertex.bitangent.z = pMesh->mBitangents[i].z;
        }
    }
}

void MMeshImporter::ProcessMeshIndices(aiMesh* pMesh, MIMesh* pMMesh)
{
    // Note: Assumes triangulated mesh (3 indices per face)
    pMMesh->CreateIndices(pMesh->mNumFaces, 3);

    for (uint32_t i = 0; i < pMesh->mNumFaces; ++i)
    {
        const aiFace& face = pMesh->mFaces[i];

        for (uint32_t j = 0; j < face.mNumIndices; ++j) { pMMesh->GetIndices()[i * 3 + j] = face.mIndices[j]; }
    }
}

void MMeshImporter::BindVertexAndBones(MSkeleton* pSkeleton, aiMesh* pMesh, MMesh<MVertexWithBones>* pMMesh)
{
    if (!pSkeleton || !pMesh->HasBones()) { return; }

    // Bind bone weights to vertices
    for (uint32_t i = 0; i < pMesh->mNumBones; ++i)
    {
        aiBone* pBone = pMesh->mBones[i];
        if (!pBone) { continue; }

        MString      strBoneName(pBone->mName.data);
        const MBone* pMBone = pSkeleton->FindBoneByName(strBoneName);
        if (!pMBone) { continue; }

        for (uint32_t wgtIndex = 0; wgtIndex < pBone->mNumWeights; ++wgtIndex)
        {
            aiVertexWeight    wgt    = pBone->mWeights[wgtIndex];
            MVertexWithBones& vertex = pMMesh->GetVertices()[wgt.mVertexId];

            // Find empty bone slot and assign weight
            for (uint32_t boneIndex = 0; boneIndex < MRenderGlobal::BONES_PER_VERTEX; ++boneIndex)
            {
                if (vertex.bonesWeight[boneIndex] == 0)
                {
                    vertex.bonesID[boneIndex]     = pMBone->unIndex;
                    vertex.bonesWeight[boneIndex] = wgt.mWeight;
                    break;
                }
            }
        }
    }

    // Normalize bone weights
    for (uint32_t i = 0; i < pMMesh->GetVerticesNum(); ++i)
    {
        MVertexWithBones& vertex = pMMesh->GetVertices()[i];

        float             fLength = 0.0f;
        for (uint32_t n = 0; n < MRenderGlobal::BONES_PER_VERTEX; ++n) { fLength += vertex.bonesWeight[n]; }

        if (fLength > 0.0f)
        {
            for (uint32_t n = 0; n < MRenderGlobal::BONES_PER_VERTEX; ++n) { vertex.bonesWeight[n] /= fLength; }
        }
    }
}

}// namespace morty
