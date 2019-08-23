#include "MModelResource.h"
#include "MModel.h"

#include "MLogManager.h"
#include "MMesh.h"
#include "MModel.h"
#include "MModelResource.h"
#include "MResourceManager.h"

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

MModelResource::MModelResource()
: MResource()
, m_pModelTemplate(nullptr)
{
    m_pModelTemplate = new MModel();
}

MModelResource::~MModelResource()
{
	delete m_pModelTemplate;
}

bool MModelResource::Load(const MString& strResourcePath)
{
	
	m_pModelTemplate->Clean();

	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(strResourcePath, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
	unsigned int* p = scene->mRootNode->mMeshes;
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
	{
		MLogManager::GetInstance()->Error("ERROR::ASSIMP:: %s", importer.GetErrorString());
		return false;
	}

	ProcessNode(scene->mRootNode, scene, m_pModelTemplate);

}


void MModelResource::ProcessNode(aiNode *pNode, const aiScene *pScene, MModel* pModel)
{
	for (unsigned int i = 0; i < pNode->mNumMeshes; ++i)
	{
		aiMesh* pMesh = pScene->mMeshes[pNode->mMeshes[i]];

		MMesh* pMMesh = new MMesh();
		ProcessMesh(pMesh, pScene, pMMesh);
		m_pModelTemplate->m_vMeshes.push_back(pMMesh);
	}

	for (unsigned int i = 0; i < pNode->mNumChildren; ++i)
	{
		ProcessNode(pNode->mChildren[i], pScene, pModel);
	}
}

void MModelResource::ProcessMesh(aiMesh* pMesh, const aiScene* pScene, MMesh* pMMesh)
{
	pMMesh->CreateVertices(pMesh->mNumVertices);
	for (unsigned int i = 0; i < pMesh->mNumVertices; ++i)
	{
		MMesh::Vertex& vertex = pMMesh->m_vVertices[i];
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

	}

	// TODO 写死3不安全，多个顶点组成一个面的模型会有危险。
	pMMesh->CreateIndices(pMesh->mNumFaces, 3);

	for (unsigned int i = 0; i < pMesh->mNumFaces; ++i)
	{
		const aiFace& face = pMesh->mFaces[i];

		for (unsigned int j = 0; j < face.mNumIndices; ++j)
		{
			pMMesh->m_vIndices[i * 3 + j] = face.mIndices[j];
		}
	}
}