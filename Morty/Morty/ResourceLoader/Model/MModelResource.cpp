#include "MModelResource.h"

#include "MLogManager.h"
#include "MMesh.h"
#include "MModelResource.h"
#include "MResourceManager.h"
#include "MVertex.h"

#include "MIDevice.h"
#include "MEngine.h"

#include "MMaterial.h"
#include "MMaterialResource.h"

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

MModelResource::MEMeshVertexType MModelResource::GetMeshVertexType(const unsigned int& unIndex)
{
	return unIndex < m_vVertexTypes.size() ? m_vVertexTypes[unIndex] : MEMeshVertexType::Normal;
}

MMaterial* MModelResource::GetMeshDefaultMaterial(const unsigned int& unIndex)
{
	return unIndex < m_vDefaultMaterial.size() ? m_vDefaultMaterial[unIndex] : nullptr;
}

void loadMaterialTextures(aiMaterial* mat, const aiTextureType& type, const MString& strTypeName)
{
	for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
	{
		aiString str;
		mat->GetTexture(type, i, &str);

		int a = 0;
		++a;
	}
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
		m_vSkeletalAnimation.clear();
		m_tSkeletalAnimation.clear();
	}
	m_pSkeleton = new MSkeleton();

	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(strResourcePath, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace | aiProcess_FixInfacingNormals | aiProcess_ConvertToLeftHanded);
	unsigned int* p = scene->mRootNode->mMeshes;
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
	{
		MLogManager::GetInstance()->Error("ERROR::ASSIMP:: %s", importer.GetErrorString());
		return false;
	}

	//顶点数组对应的材质索引
	std::vector<unsigned int> vMaterialIndices;

	ProcessBones(scene);
	Matrix4 matRootRotation = Matrix4::IdentityMatrix;
	ProcessNode(scene->mRootNode, scene, vMaterialIndices, matRootRotation);
	ProcessAnimation(scene);
	ProcessMaterial(scene, vMaterialIndices);
	
	return true;
}

void MModelResource::ProcessNode(aiNode *pNode, const aiScene *pScene, std::vector<unsigned int>& vMaterialIndices, const Matrix4& matParentRotation)
{
	Matrix4 matRotation;
	CopyMatrix4(&matRotation, &pNode->mTransformation);
	matRotation = matParentRotation * matRotation.GetRotatePart();

	for (unsigned int i = 0; i < pNode->mNumMeshes; ++i)
	{
		aiMesh* pMesh = pScene->mMeshes[pNode->mMeshes[i]];

		if (pMesh->HasBones())
		{
			MMesh<MVertexWithBones>* pMMesh = new MMesh<MVertexWithBones>();
			ProcessMeshVertices(pMesh, pScene, pMMesh, matRotation);
			ProcessMeshIndices(pMesh, pScene, pMMesh);
			BindVertexAndBones(pMesh, pScene, pMMesh);
			m_vMeshes.push_back(pMMesh);
			m_vVertexTypes.push_back(MEMeshVertexType::Skeleton);
			vMaterialIndices.push_back(pMesh->mMaterialIndex);
			m_vDefaultMaterial.push_back(nullptr);
		}
		else
		{
			MMesh<MVertex>* pMMesh = new MMesh<MVertex>();
			ProcessMeshVertices(pMesh, pScene, pMMesh, matRotation);
			ProcessMeshIndices(pMesh, pScene, pMMesh);
			m_vMeshes.push_back(pMMesh);
			m_vVertexTypes.push_back(MEMeshVertexType::Normal);
			vMaterialIndices.push_back(pMesh->mMaterialIndex);
			m_vDefaultMaterial.push_back(nullptr);
		}
	}

	for (unsigned int i = 0; i < pNode->mNumChildren; ++i)
	{
		ProcessNode(pNode->mChildren[i], pScene, vMaterialIndices, matRotation);
	}
}

void MModelResource::ProcessMeshVertices(aiMesh* pMesh, const aiScene* pScene, MMesh<MVertex>* pMMesh, const Matrix4& matRotation)
{
	pMMesh->CreateVertices(pMesh->mNumVertices);
	for (unsigned int i = 0; i < pMesh->mNumVertices; ++i)
	{
		MVertex& vertex = pMMesh->GetVertices()[i];
		vertex.position.x = pMesh->mVertices[i].x;
		vertex.position.y = pMesh->mVertices[i].y;
		vertex.position.z = pMesh->mVertices[i].z;

		vertex.position = matRotation * vertex.position;

		if (pMesh->mNormals)
		{
			vertex.normal.x = pMesh->mNormals[i].x;
			vertex.normal.y = pMesh->mNormals[i].y;
			vertex.normal.z = pMesh->mNormals[i].z;

			vertex.normal = matRotation * vertex.normal;
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

			vertex.tangent = matRotation * vertex.tangent;
		}
		if (pMesh->mBitangents)
		{
			vertex.bitangent.x = pMesh->mBitangents[i].x;
			vertex.bitangent.y = pMesh->mBitangents[i].y;
			vertex.bitangent.z = pMesh->mBitangents[i].z;

			vertex.bitangent = matRotation * vertex.bitangent;
		}
	}
}

void MModelResource::ProcessMeshVertices(aiMesh* pMesh, const aiScene* pScene, MMesh<MVertexWithBones>* pMMesh, const Matrix4& matRotation)
{
	pMMesh->CreateVertices(pMesh->mNumVertices);
	for (unsigned int i = 0; i < pMesh->mNumVertices; ++i)
	{
		MVertexWithBones& vertex = pMMesh->GetVertices()[i];
		vertex.position.x = pMesh->mVertices[i].x;
		vertex.position.y = pMesh->mVertices[i].y;
		vertex.position.z = pMesh->mVertices[i].z;

		vertex.position = matRotation * vertex.position;

		if (pMesh->mNormals)
		{
			vertex.normal.x = pMesh->mNormals[i].x;
			vertex.normal.y = pMesh->mNormals[i].y;
			vertex.normal.z = pMesh->mNormals[i].z;

			vertex.normal = matRotation * vertex.normal;
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

			vertex.tangent = matRotation * vertex.tangent;
		}
		if (pMesh->mBitangents)
		{
			vertex.bitangent.x = pMesh->mBitangents[i].x;
			vertex.bitangent.y = pMesh->mBitangents[i].y;
			vertex.bitangent.z = pMesh->mBitangents[i].z;

			vertex.bitangent = matRotation * vertex.bitangent;
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

void MModelResource::BindVertexAndBones(aiMesh* pMesh, const aiScene* pScene, MMesh<MVertexWithBones>* pMMesh)
{
	for (unsigned int i = 0; i < pMesh->mNumBones; ++i)
	{
		if (aiBone* pBone = pMesh->mBones[i])
		{
			MString strBoneName(pBone->mName.data); 
			if (MBone* pMBone = m_pSkeleton->FindBoneByName(strBoneName))
			{
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
}

void MModelResource::RecordBones(aiNode* pNode, const aiScene* pScene)
{
	for (unsigned int i = 0; i < pNode->mNumMeshes; ++i)
	{
		aiMesh* pMesh = pScene->mMeshes[pNode->mMeshes[i]];
		if (pMesh->HasBones())
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
				}
			}
		}
	}

	for (unsigned int i = 0; i < pNode->mNumChildren; ++i)
	{
		RecordBones(pNode->mChildren[i], pScene);
	}
}

void MModelResource::ProcessBones(const aiScene* pScene)
{
	RecordBones(pScene->mRootNode, pScene);
	BindBones(pScene->mRootNode, pScene);
	m_pSkeleton->SortByDeep();
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
		m_vSkeletalAnimation.push_back(pAnimation->mName.C_Str());
		m_tSkeletalAnimation[pAnimation->mName.C_Str()] = pMAnimation;

		pMAnimation->m_strName = pAnimation->mName.C_Str();
		pMAnimation->m_fTicksDuration = pAnimation->mDuration;
		if(pAnimation->mTicksPerSecond > 0.0f)
			pMAnimation->m_fTicksPerSecond = pAnimation->mTicksPerSecond;

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

void MModelResource::ProcessMaterial(const aiScene* pScene, std::vector<unsigned int>& vMaterialIndices)
{
	aiString strAmbient("$clr.ambient"), strDiffuse("$clr.diffuse"), strSpecular("$clr.specular"), strShininess("$mat.shininess");

	MMaterialResource* pStaticMeshMaterial = m_pEngine->GetResourceManager()->LoadVirtualResource<MMaterialResource>(DEFAULT_MATERIAL_STATIC);
	MMaterialResource* pSkinnedMeshMaterial = m_pEngine->GetResourceManager()->LoadVirtualResource<MMaterialResource>(DEFAULT_MATERIAL_SKINNED);

	for (unsigned int i = 0; i < m_vVertexTypes.size(); ++i)
	{
		if (m_vVertexTypes[i] == MEMeshVertexType::Normal)
		{
			m_vDefaultMaterial[i] = m_pEngine->GetObjectManager()->CreateObject<MMaterial>();
			m_vDefaultMaterial[i]->Load(pStaticMeshMaterial);
		}
		else if(m_vVertexTypes[i] == MEMeshVertexType::Skeleton)
		{
			m_vDefaultMaterial[i] = m_pEngine->GetObjectManager()->CreateObject<MMaterial>();
			m_vDefaultMaterial[i]->Load(pSkinnedMeshMaterial);
		}

		unsigned int unMaterialIndex = vMaterialIndices[i];

		if(unMaterialIndex >= pScene->mNumMaterials)
			continue;

		aiMaterial* pMaterial = pScene->mMaterials[unMaterialIndex];

		Vector3 v3Ambient(1.0f, 1.0f, 1.0f), v3Diffuse(1.0f, 1.0f, 1.0f), v3Specular(1.0f, 1.0f, 1.0f);
		float fShininess = 32.0f;
		for (int n = 0; n < pMaterial->mNumProperties; ++n)
		{
			aiMaterialProperty* prop =  pMaterial->mProperties[n];
			if (prop->mKey == strAmbient)
				memcpy(v3Ambient.m, prop->mData, sizeof(Vector3));
			else if (prop->mKey == strDiffuse)
				memcpy(v3Diffuse.m, prop->mData, sizeof(Vector3));
			else if (prop->mKey == strSpecular)
				memcpy(v3Specular.m, prop->mData, sizeof(Vector3));
			else if (prop->mKey == strShininess)
				memcpy(&fShininess, prop->mData, sizeof(float));
		}
		//Test Data, need read from file.
		for (MShaderParam& param : m_vDefaultMaterial[i]->GetPixelShaderParams())
		{
			if (param.strName == "cbMaterial")
			{
				if (MStruct* pStruct = param.var.GetByType<MStruct>())
				{
					if (MStruct* pMat = pStruct->FindMember("U_mat")->GetByType<MStruct>())
					{
						pMat->SetMember("f3Ambient", v3Ambient);
						pMat->SetMember("f3Diffuse", v3Diffuse);
						pMat->SetMember("f3Specular", v3Specular);
						pMat->SetMember("fShininess", fShininess);
					}
				}
			}
		}

		if (MResource* pTexResource = m_pEngine->GetResourceManager()->LoadResource("./Model/teaport.png"))
		{
			m_vDefaultMaterial[i]->SetPixelTexutreParam("U_mat.texDiffuse", pTexResource);
		//	pMaterial->SetPixelTexutreParam("U_mat.texSpecular", pTexResource);
		}
	}
}
