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
#include "Tools/MClusterBuilder.h"
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

        pMeshResourceData->mesh.reset(pBonesMesh);
        pMeshResourceData->eVertexType = MEMeshVertexType::Skeleton;
    }
    else
    {
        // Static mesh
        auto* pStaticMesh = new MMesh<MVertex>();
        ProcessMeshVertices(pAiMesh, pStaticMesh);
        ProcessMeshIndices(pAiMesh, pStaticMesh);

        // Generate clusters if Nanite is enabled (only for static meshes)
        if (m_enableNanite)
        {
            MClusterBuilder builder;
            builder.SetQuality(MEClusterQuality::Low);
            builder.Generate(pStaticMesh);
        }

        pMeshResourceData->mesh.reset(pStaticMesh);
        pMeshResourceData->eVertexType = MEMeshVertexType::Normal;
    }

    // Create mesh resource
    std::shared_ptr<MMeshResource> pMeshResource = m_resourceSystem->CreateResource<MMeshResource>();
    pMeshResource->Load(std::move(pResourceData));
    pMeshResource->ResetBounds();

    m_cache[pAiMesh] = pMeshResource;
    return pMeshResource;
}

void MMeshImporter::ProcessMeshVertices(aiMesh* mesh, MMesh<MVertex>* pMMesh)
{
    pMMesh->CreateVertices(mesh->mNumVertices);

    for (uint32_t i = 0; i < mesh->mNumVertices; ++i)
    {
        MVertex& vertex = pMMesh->GetVertices()[i];

        // Position
        vertex.position.x = mesh->mVertices[i].x;
        vertex.position.y = mesh->mVertices[i].y;
        vertex.position.z = mesh->mVertices[i].z;

        // Normal
        if (mesh->mNormals)
        {
            vertex.normal.x = mesh->mNormals[i].x;
            vertex.normal.y = mesh->mNormals[i].y;
            vertex.normal.z = mesh->mNormals[i].z;
        }

        // Texture coordinates
        if (mesh->mTextureCoords[0])
        {
            vertex.texCoords.x = mesh->mTextureCoords[0][i].x;
            vertex.texCoords.y = mesh->mTextureCoords[0][i].y;
        }

        // Tangent
        if (mesh->mTangents)
        {
            vertex.tangent.x = mesh->mTangents[i].x;
            vertex.tangent.y = mesh->mTangents[i].y;
            vertex.tangent.z = mesh->mTangents[i].z;
        }

        // Bitangent
        if (mesh->mBitangents)
        {
            vertex.bitangent.x = mesh->mBitangents[i].x;
            vertex.bitangent.y = mesh->mBitangents[i].y;
            vertex.bitangent.z = mesh->mBitangents[i].z;
        }
    }
}

void MMeshImporter::ProcessMeshVertices(aiMesh* mesh, MMesh<MVertexWithBones>* pMMesh)
{
    pMMesh->CreateVertices(mesh->mNumVertices);

    for (uint32_t i = 0; i < mesh->mNumVertices; ++i)
    {
        MVertexWithBones& vertex = pMMesh->GetVertices()[i];

        // Position
        vertex.position.x = mesh->mVertices[i].x;
        vertex.position.y = mesh->mVertices[i].y;
        vertex.position.z = mesh->mVertices[i].z;

        // Normal
        if (mesh->mNormals)
        {
            vertex.normal.x = mesh->mNormals[i].x;
            vertex.normal.y = mesh->mNormals[i].y;
            vertex.normal.z = mesh->mNormals[i].z;
        }

        // Texture coordinates
        if (mesh->mTextureCoords[0])
        {
            vertex.texCoords.x = mesh->mTextureCoords[0][i].x;
            vertex.texCoords.y = mesh->mTextureCoords[0][i].y;
        }

        // Tangent
        if (mesh->mTangents)
        {
            vertex.tangent.x = mesh->mTangents[i].x;
            vertex.tangent.y = mesh->mTangents[i].y;
            vertex.tangent.z = mesh->mTangents[i].z;
        }

        // Bitangent
        if (mesh->mBitangents)
        {
            vertex.bitangent.x = mesh->mBitangents[i].x;
            vertex.bitangent.y = mesh->mBitangents[i].y;
            vertex.bitangent.z = mesh->mBitangents[i].z;
        }
    }
}

void MMeshImporter::ProcessMeshIndices(aiMesh* mesh, MIMesh* pMMesh)
{
    // Note: Assumes triangulated mesh (3 indices per face)
    pMMesh->CreateIndices(mesh->mNumFaces, 3);

    for (uint32_t i = 0; i < mesh->mNumFaces; ++i)
    {
        const aiFace& face = mesh->mFaces[i];

        for (uint32_t j = 0; j < face.mNumIndices; ++j) { pMMesh->GetIndices()[i * 3 + j] = face.mIndices[j]; }
    }
}

void MMeshImporter::BindVertexAndBones(MSkeleton* pSkeleton, aiMesh* mesh, MMesh<MVertexWithBones>* pMMesh)
{
    if (!pSkeleton || !mesh->HasBones()) { return; }

    // Bind bone weights to vertices
    for (uint32_t i = 0; i < mesh->mNumBones; ++i)
    {
        aiBone* pBone = mesh->mBones[i];
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
