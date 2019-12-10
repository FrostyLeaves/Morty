#include "MModelResource.h"

#include "MLogManager.h"
#include "MMesh.h"
#include "MModelResource.h"
#include "MResourceManager.h"
#include "MVertex.h"

#include "MIDevice.h"
#include "MEngine.h"

#include "MBounds.h"
#include "MSkeleton.h"
#include "MSkeletalAnimation.h"

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"


void CopyMatrix4(Matrix4* matdest, aiMatrix4x4* matsour)
{
	for (unsigned int r = 0; r < 4; ++r)
	{
		for (unsigned int c = 0; c < 4; ++c)
		{
			matdest->m[r][c] = (*matsour)[r][c];
		}
	}
}


MModelResource::MModelResource()
: MResource()
, m_vMeshes()
, m_pBoundsOBB(nullptr)
, m_pSkeleton(nullptr)
{
}

MModelResource::~MModelResource()
{
	if (m_pBoundsOBB)
	{
		delete m_pBoundsOBB;
		m_pBoundsOBB = nullptr;
	}

	if (m_pSkeleton)
	{
		delete m_pSkeleton;
		m_pSkeleton = nullptr;
	}

	for (MIMesh* pMesh : m_vMeshes)
	{
		pMesh->DestroyBuffer(m_pEngine->GetDevice());
		delete pMesh;
	}
	m_vMeshes.clear();
}

const MBoundsOBB* MModelResource::GetOBB()
{
	std::vector<Vector3> vPoints;
	if (nullptr == m_pBoundsOBB)
	{
		for (MIMesh* pMesh : m_vMeshes)
		{
			if (MMesh<MVertex>* pMeshIns = dynamic_cast<MMesh<MVertex>*>(pMesh))
			{
				for (unsigned int i = 0; i < pMesh->GetVerticesLength(); ++i)
				{
					vPoints.push_back(pMeshIns->GetVertices()[i].position);
				}
			}
			else if(MMesh<MVertexWithBones>* pMeshIns = dynamic_cast<MMesh<MVertexWithBones>*>(pMesh))
			{
				for (unsigned int i = 0; i < pMesh->GetVerticesLength(); ++i)
				{
					vPoints.push_back(pMeshIns->GetVertices()[i].position);
				}
			}
		}
		m_pBoundsOBB = new MBoundsOBB(vPoints);
	}

	return m_pBoundsOBB;
}

bool MModelResource::Load(const MString& strResourcePath)
{
	for (MIMesh* pMesh : m_vMeshes)
	{
		pMesh->DestroyBuffer(m_pEngine->GetDevice());
		delete pMesh;
	}
	m_vMeshes.clear(); 

	if (m_pSkeleton)
	{
		delete m_pSkeleton;
		m_pSkeleton = nullptr;
	}
	m_pSkeleton = new MSkeleton();

	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(strResourcePath, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
	unsigned int* p = scene->mRootNode->mMeshes;
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
	{
		MLogManager::GetInstance()->Error("ERROR::ASSIMP:: %s", importer.GetErrorString());
		return false;
	}

	ProcessNode(scene->mRootNode, scene);
	ProcessBones(scene);
	
	ProcessAnimation(scene);

	return true;
}

void MModelResource::ProcessNode(aiNode *pNode, const aiScene *pScene)
{
	for (unsigned int i = 0; i < pNode->mNumMeshes; ++i)
	{
		aiMesh* pMesh = pScene->mMeshes[pNode->mMeshes[i]];

		if (pMesh->HasBones())
		{
			MMesh<MVertexWithBones>* pMMesh = new MMesh<MVertexWithBones>();
			ProcessMeshVertices(pMesh, pScene, pMMesh);
			ProcessMeshIndices(pMesh, pScene, pMMesh);
			RecordBones(pMesh, pScene, pMMesh);
			m_vMeshes.push_back(pMMesh);
			m_vVertexTypes.push_back(MEModelVertexType::Skeleton);
		}
		else
		{
			MMesh<MVertex>* pMMesh = new MMesh<MVertex>();
			ProcessMeshVertices(pMesh, pScene, pMMesh);
			ProcessMeshIndices(pMesh, pScene, pMMesh);
			m_vMeshes.push_back(pMMesh);
			m_vVertexTypes.push_back(MEModelVertexType::Normal);
		}
	}

	for (unsigned int i = 0; i < pNode->mNumChildren; ++i)
	{
		ProcessNode(pNode->mChildren[i], pScene);
	}
}

void MModelResource::ProcessMeshVertices(aiMesh* pMesh, const aiScene* pScene, MMesh<MVertex>* pMMesh)
{
	pMMesh->CreateVertices(pMesh->mNumVertices);
	for (unsigned int i = 0; i < pMesh->mNumVertices; ++i)
	{
		MVertex& vertex = pMMesh->GetVertices()[i];
		vertex.position.x = pMesh->mVertices[i].x;
		vertex.position.y = pMesh->mVertices[i].y;
		vertex.position.z = pMesh->mVertices[i].z;

		if (pMesh->mNormals)
		{
			vertex.normal.x = pMesh->mNormals[i].x;
			vertex.normal.y = pMesh->mNormals[i].y;
			vertex.normal.z = pMesh->mNormals[i].z;
		}
		if (pMesh->mTextureCoords)
		{
			vertex.texCoords.x = pMesh->mTextureCoords[0][i].x;
			vertex.texCoords.y = pMesh->mTextureCoords[0][i].y;
		}

		if (pMesh->mTangents)
		{
			vertex.tangent.x = pMesh->mTangents[i].x;
			vertex.tangent.y = pMesh->mTangents[i].y;
			vertex.tangent.z = pMesh->mTangents[i].z;
		}
		if (pMesh->mBitangents)
		{
			vertex.bitangent.x = pMesh->mBitangents[i].x;
			vertex.bitangent.y = pMesh->mBitangents[i].y;
			vertex.bitangent.z = pMesh->mBitangents[i].z;
		}
	}
}

void MModelResource::ProcessMeshVertices(aiMesh* pMesh, const aiScene* pScene, MMesh<MVertexWithBones>* pMMesh)
{
	pMMesh->CreateVertices(pMesh->mNumVertices);
	for (unsigned int i = 0; i < pMesh->mNumVertices; ++i)
	{
		MVertexWithBones& vertex = pMMesh->GetVertices()[i];
		vertex.position.x = pMesh->mVertices[i].x;
		vertex.position.y = pMesh->mVertices[i].y;
		vertex.position.z = pMesh->mVertices[i].z;

		if (pMesh->mNormals)
		{
			vertex.normal.x = pMesh->mNormals[i].x;
			vertex.normal.y = pMesh->mNormals[i].y;
			vertex.normal.z = pMesh->mNormals[i].z;
		}
		if (pMesh->mTextureCoords)
		{
			vertex.texCoords.x = pMesh->mTextureCoords[0][i].x;
			vertex.texCoords.y = pMesh->mTextureCoords[0][i].y;
		}

		if (pMesh->mTangents)
		{
			vertex.tangent.x = pMesh->mTangents[i].x;
			vertex.tangent.y = pMesh->mTangents[i].y;
			vertex.tangent.z = pMesh->mTangents[i].z;
		}
		if (pMesh->mBitangents)
		{
			vertex.bitangent.x = pMesh->mBitangents[i].x;
			vertex.bitangent.y = pMesh->mBitangents[i].y;
			vertex.bitangent.z = pMesh->mBitangents[i].z;
		}
	}
}

void MModelResource::ProcessMeshIndices(aiMesh* pMesh, const aiScene* pScene, MIMesh* pMMesh)
{
	// TODO 写死3不安全，多个顶点组成一个面的模型会有危险。
	pMMesh->CreateIndices(pMesh->mNumFaces, 3);

	for (unsigned int i = 0; i < pMesh->mNumFaces; ++i)
	{
		const aiFace& face = pMesh->mFaces[i];

		for (unsigned int j = 0; j < face.mNumIndices; ++j)
		{
			pMMesh->GetIndices()[i * 3 + j] = face.mIndices[j];
		}
	}
}

void MModelResource::RecordBones(aiMesh* pMesh, const aiScene* pScene, MMesh<MVertexWithBones>* pMMesh)
{
	for (unsigned int i = 0; i < pMesh->mNumBones; ++i)
	{
		if (aiBone* pBone = pMesh->mBones[i])
		{
			MString strBoneName(pBone->mName.data); 
			MBone* pMBone = m_pSkeleton->FindBoneByName(strBoneName);
			if (nullptr == pMBone)
			{
				pMBone = m_pSkeleton->AppendBone(strBoneName);
				CopyMatrix4(&pMBone->m_matOffsetMatrix, &pBone->mOffsetMatrix);
			}

			for (unsigned int wgtIndex = 0; wgtIndex < pBone->mNumWeights; ++wgtIndex)
			{
				aiVertexWeight wgt = pBone->mWeights[wgtIndex];
				MVertexWithBones& vertex = pMMesh->GetVertices()[wgt.mVertexId];

				for (unsigned int boneIndex = 0; boneIndex < MBONES_PER_VERTEX; ++boneIndex)
				{
					if (0 == vertex.bonesWeight[boneIndex])
					{
						vertex.bonesID[boneIndex] = pMBone->unIndex;
						vertex.bonesWeight[boneIndex] = wgt.mWeight;
						break;
					}
				}
			}
		}
	}
}

void MModelResource::ProcessBones(const aiScene* pScene)
{
	BindBones(pScene->mRootNode, pScene);

	std::vector<int> map(m_pSkeleton->m_vAllBones.size());
	std::vector<MBone*> vBones = m_pSkeleton->m_vAllBones;
	std::map<MBone*, int> tDeep;

	for (MBone* pBone : m_pSkeleton->m_vAllBones)
	{
		int deep = 0;
		MBone* pParent = pBone;
		while (pParent->unParentIndex != MBone::InvalidIndex)
		{
			pParent = m_pSkeleton->m_vAllBones[pParent->unParentIndex];
			++deep;
		}

		tDeep[pBone] = deep;
	}

	std::sort(vBones.begin(), vBones.end(), [&tDeep](MBone* a, MBone* b) { return tDeep[a] < tDeep[b]; });

	for (unsigned int i = 0; i < vBones.size(); ++i)
		map[vBones[i]->unIndex] = i;

	for (unsigned int i = 0; i < vBones.size(); ++i)
	{
		MBone* pBone = vBones[i];
		pBone->unIndex = map[pBone->unIndex];
		for (unsigned int& index : pBone->vChildrenIndices)
			index = map[index];
	}

	for (auto& iter : m_pSkeleton->m_tBonesMap)
		iter.second = map[iter.second];

	m_pSkeleton->m_vAllBones = vBones;

	for (MIMesh* pMesh : m_vMeshes)
	{
		if (MMesh<MVertexWithBones>* pMMesh = dynamic_cast<MMesh<MVertexWithBones>*>(pMesh))
		{
			for (unsigned int i = 0; i < pMMesh->GetVerticesLength(); ++i)
			{
				MVertexWithBones& vertex = pMMesh->GetVertices()[i];
				for (unsigned int boneIndex = 0; boneIndex < MBONES_PER_VERTEX; ++boneIndex)
				{
					if (0 != vertex.bonesWeight[boneIndex])
					{
						vertex.bonesID[boneIndex] = map[vertex.bonesID[boneIndex]];
					}
				}
			}
		}
	}
}

void MModelResource::BindBones(aiNode* pNode, const aiScene* pScene, MBone* pParent/* = nullptr*/)
{
	MBone* pMBone = nullptr;
	if (pMBone = m_pSkeleton->FindBoneByName(pNode->mName.data))
	{
		if (pParent)
		{
			pMBone->unParentIndex = pParent->unIndex;
			pParent->vChildrenIndices.push_back(pMBone->unIndex);
		}
		else
		{
			pMBone->unParentIndex = MBone::InvalidIndex;
		}

		CopyMatrix4(&pMBone->m_matTransform, &pNode->mTransformation);
	}

	for (unsigned int i = 0; i < pNode->mNumChildren; ++i)
	{
		BindBones(pNode->mChildren[i], pScene, pMBone);
	}
}

void MModelResource::ProcessAnimation(const aiScene* pScene)
{
	for (unsigned int i = 0; i < pScene->mNumAnimations; ++i)
	{
		aiAnimation* pAnimation = pScene->mAnimations[i];
		MSkeletalAnimation* pMAnimation = new MSkeletalAnimation();
		pMAnimation->m_pSkeletonTemplate = m_pSkeleton;
		pMAnimation->m_vSkeletalAnimNodes.resize(pAnimation->mNumChannels);// init by nullptr

		//Record
		m_tSkeletalAnimation[pAnimation->mName.C_Str()] = pMAnimation;

		pMAnimation->m_strName = pAnimation->mName.C_Str();
		pMAnimation->m_fDuration = pAnimation->mDuration;

		for (unsigned int chanIndex = 0; chanIndex < pAnimation->mNumChannels; ++chanIndex)
		{
			aiNodeAnim* pNodeAnim = pAnimation->mChannels[chanIndex];

			if (MBone* pBone = m_pSkeleton->FindBoneByName(pNodeAnim->mNodeName.C_Str()))
			{
				MSkeletalAnimNode* pMAnimNode = new MSkeletalAnimNode();
				pMAnimation->m_vSkeletalAnimNodes[pBone->unIndex] = pMAnimNode;
				
				pMAnimNode->m_unPositionKeysNum = pNodeAnim->mNumPositionKeys;
				pMAnimNode->m_unRotationKeysNum = pNodeAnim->mNumRotationKeys;
				pMAnimNode->m_unScalingKeysNum = pNodeAnim->mNumScalingKeys;
				
				if (pMAnimNode->m_unPositionKeysNum > 0)
				{
					pMAnimNode->m_vPositionKeys = new MSkeletalAnimNode::MAnimNodeKey<Vector3>[pMAnimNode->m_unPositionKeysNum];
					for (unsigned keyIndex = 0; keyIndex < pMAnimNode->m_unPositionKeysNum; ++keyIndex)
					{
						MSkeletalAnimNode::MAnimNodeKey<Vector3>& dkey = pMAnimNode->m_vPositionKeys[keyIndex];
						const aiVectorKey& skey = pNodeAnim->mPositionKeys[keyIndex];
						dkey.mTime = skey.mTime;
						dkey.mValue.x = skey.mValue.x;
						dkey.mValue.y = skey.mValue.y;
						dkey.mValue.z = skey.mValue.z;
					}
				}
				if (pMAnimNode->m_unRotationKeysNum > 0)
				{
					pMAnimNode->m_vRotationKeys = new MSkeletalAnimNode::MAnimNodeKey<Quaternion>[pMAnimNode->m_unRotationKeysNum];
					for (unsigned keyIndex = 0; keyIndex < pMAnimNode->m_unRotationKeysNum; ++keyIndex)
					{
						MSkeletalAnimNode::MAnimNodeKey<Quaternion>& dkey = pMAnimNode->m_vRotationKeys[keyIndex];
						const aiQuatKey& skey = pNodeAnim->mRotationKeys[keyIndex];
						dkey.mTime = skey.mTime;
						dkey.mValue.w = skey.mValue.w;
						dkey.mValue.x = skey.mValue.x;
						dkey.mValue.y = skey.mValue.y;
						dkey.mValue.z = skey.mValue.z;
					}
				}
				if (pMAnimNode->m_unScalingKeysNum > 0)
				{
					pMAnimNode->m_vScalingKeys = new MSkeletalAnimNode::MAnimNodeKey<Vector3>[pMAnimNode->m_unScalingKeysNum];
					for (unsigned keyIndex = 0; keyIndex < pMAnimNode->m_unScalingKeysNum; ++keyIndex)
					{
						MSkeletalAnimNode::MAnimNodeKey<Vector3>& dkey = pMAnimNode->m_vScalingKeys[keyIndex];
						const aiVectorKey& skey = pNodeAnim->mScalingKeys[keyIndex];
						dkey.mTime = skey.mTime;
						dkey.mValue.x = skey.mValue.x;
						dkey.mValue.y = skey.mValue.y;
						dkey.mValue.z = skey.mValue.z;
					}
				}
			}
		}
	}
}
