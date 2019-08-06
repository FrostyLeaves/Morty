#include "MModelLoader.h"
#include "MLogManager.h"
#include "MMesh.h"
#include "MModel.h"
#include "MModelResource.h"

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

MModelLoader::MModelLoader()
: MResourceLoader()
{
    
}

MModelLoader::~MModelLoader()
{
    
}

MResource* MModelLoader::Load(const char *svPath)
{
    // read file via ASSIMP
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(svPath, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
    // check for errors
    if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
    {
        MLogManager::GetInstance()->Error("ERROR::ASSIMP:: %s", importer.GetErrorString());
        return nullptr;
    }

    MModelResource* pResource = MResourceManager::GetInstance()->CreateResource<MModelResource>();
    
    // process ASSIMP's root node recursively
    ProcessNode(scene->mRootNode, scene, pResource->m_pModelTemplate);
    
    return pResource;
}

void MModelLoader::ProcessNode(aiNode *pNode, const aiScene *pScene, MModel* pModel)
{
    for (unsigned int i = 0; i < pNode->mNumMeshes; ++i)
    {
        aiMesh* pMesh = pScene->mMeshes[pNode->mMeshes[i]];
    
        MMesh* pMMesh = new MMesh();
        ProcessMesh(pMesh, pScene, pMMesh);
        pModel->m_vMeshes.push_back(pMMesh);
    }
    
    for (unsigned int i = 0; i < pNode->mNumChildren; ++i)
    {
        ProcessNode(pNode->mChildren[i], pScene, pModel);
    }
}

void MModelLoader::ProcessMesh(aiMesh* pMesh, const aiScene* pScene, MMesh* pMMesh)
{
    for (unsigned int i = 0; i < pMesh->mNumVertices; ++i)
    {
        MMesh::Vertex vertex;
        vertex.position.x = pMesh->mVertices[i].x;
        vertex.position.y = pMesh->mVertices[i].y;
        vertex.position.z = pMesh->mVertices[i].z;
        
        vertex.normal.x = pMesh->mNormals[i].x;
        vertex.normal.y = pMesh->mNormals[i].y;
        vertex.normal.z = pMesh->mNormals[i].z;
        
        if (pMesh->mTextureCoords[0])
        {
            vertex.texCoords.x = pMesh->mTextureCoords[0][i].x;
            vertex.texCoords.y = pMesh->mTextureCoords[0][i].y;
        }
        
        vertex.tangent.x = pMesh->mTangents[i].x;
        vertex.tangent.y = pMesh->mTangents[i].y;
        vertex.tangent.z = pMesh->mTangents[i].z;
        
        vertex.bitangent.x = pMesh->mBitangents[i].x;
        vertex.bitangent.y = pMesh->mBitangents[i].y;
        vertex.bitangent.z = pMesh->mBitangents[i].z;
        
        pMMesh->m_vVertices.push_back(vertex);
    }
    
    for (unsigned int i = 0; i < pMesh->mNumFaces; ++i)
    {
        const aiFace& face = pMesh->mFaces[i];
        
        for (unsigned int j = 0; j < face.mNumIndices; ++j)
        {
            pMMesh->m_vIndices.push_back(face.mIndices[j]);
        }
    }
}
