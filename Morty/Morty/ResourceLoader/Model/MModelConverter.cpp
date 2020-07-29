#include "MModelConverter.h"

#include "MLogManager.h"
#include "MMesh.h"
#include "MResourceManager.h"
#include "MVertex.h"
#include "MMeshResource.h"

#include "MIDevice.h"
#include "MEngine.h"

#include "Model/MModelResource.h"
#include "Model/MSkeletonResource.h"
#include "Texture/MTextureResource.h"
#include "Material/MMaterialResource.h"
#include "Model/MSkeletalAnimationResource.h"

#include "MBounds.h"
#include "MSkeleton.h"
#include "MSkeletalAnimation.h"

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#include "MFileHelper.h"

#include "Timer/MTimer.h"

void CopyMatrix4(Matrix4* matdest, aiMatrix4x4* matsour)
{
	for (uint32_t r = 0; r < 4; ++r)
	{
		for (uint32_t c = 0; c < 4; ++c)
		{
			matdest->m[r][c] = (*matsour)[r][c];
		}
	}
}

MModelConverter::MModelConverter(MEngine* pEngine)
: m_pEngine(pEngine)
, m_strResourcePath()
, m_vMeshes()
, m_pSkeleton()
, m_vSkeletalAnimation()
{
}

MModelConverter::~MModelConverter()
{
	if (m_pSkeleton)
	{
		m_pSkeleton->SubRef();
		m_pSkeleton = nullptr;
	}

	for (MMeshResource* pMeshRes : m_vMeshes)
		pMeshRes->SubRef();
	
	m_vMeshes.clear();

	for (MSkeletalAnimationResource* pAnim : m_vSkeletalAnimation)
		pAnim->SubRef();

	m_vSkeletalAnimation.clear();
}

bool MModelConverter::Convert(const MString& strResourcePath, const MString& strOutputDir, const MString& strOutputName)
{
	m_strResourcePath = strResourcePath;

	auto time = MTimer::GetCurTime();
	if (!Load(strResourcePath))
		return false;

	time = MTimer::GetCurTime() - time;

	MLogManager::GetInstance()->Log("Load Model Time: %lld", time);

	MString strPath = strOutputDir + "/" + strOutputName + "/";

	MFileHelper::MakeDir(strOutputDir + "/" + strOutputName);

	if (m_pSkeleton)
	{
		m_pEngine->GetResourceManager()->MoveTo(m_pSkeleton, strPath + strOutputName + "." + SUFFIX_SKELETON);
		m_pSkeleton->Save();
	}
	for (int i = 0; i < m_vMeshes.size(); ++i)
	{
		MMeshResource* pMeshResource = m_vMeshes[i];
		MString strMeshFileName = strPath + pMeshResource->GetMeshName() + "_" + MStringHelper::ToString(i);

		if (MResource* pMatRes = pMeshResource->GetDefaultMaterial())
		{
			if (MMaterialResource* pMaterialResource = pMatRes->DynamicCast<MMaterialResource>())
			{
				m_pEngine->GetResourceManager()->MoveTo(pMaterialResource, strMeshFileName + "." + SUFFIX_MATERIAL);
				pMaterialResource->Save();
			}
		}

		m_pEngine->GetResourceManager()->MoveTo(pMeshResource, strMeshFileName + "." + SUFFIX_MESH);
		pMeshResource->Save();
	}

	for (MSkeletalAnimation* pAnimResource : m_vSkeletalAnimation)
	{
		MString strValidFileName = pAnimResource->GetName();
		MFileHelper::GetValidFileName(strValidFileName);
		m_pEngine->GetResourceManager()->MoveTo(pAnimResource, strPath + strValidFileName + "." + SUFFIX_SKELANIM);
		pAnimResource->Save();

	}

	MModelResource* pModelResource = m_pEngine->GetResourceManager()->CreateResource<MModelResource>();
	pModelResource->AddRef();
	pModelResource->SetSkeletonResource(m_pSkeleton);
	pModelResource->GetMeshResources(m_vMeshes);

	m_pEngine->GetResourceManager()->MoveTo(pModelResource, strPath + strOutputName + "." + SUFFIX_MODEL);
	pModelResource->Save();

	pModelResource->SubRef();
	pModelResource = nullptr;

	return true;
}

bool MModelConverter::Load(const MString& strResourcePath)
{
	for (MMeshResource* pMesh : m_vMeshes)
		pMesh->SubRef();
	m_vMeshes.clear();

	m_pSkeleton = m_pEngine->GetResourceManager()->CreateResource<MSkeletonResource>();
	m_pSkeleton->AddRef();

	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(strResourcePath, aiProcess_JoinIdenticalVertices | aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace | aiProcess_FixInfacingNormals | aiProcess_ConvertToLeftHanded);
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
	{
		MLogManager::GetInstance()->Error("ERROR::ASSIMP:: %s", importer.GetErrorString());
		return false;
	}

	//顶点数组对应的材质索引
	std::vector<uint32_t> vMaterialIndices;

	ProcessBones(scene);
	Matrix4 matRootRotation = Matrix4::IdentityMatrix;
	ProcessNode(scene->mRootNode, scene, vMaterialIndices, matRootRotation);
	ProcessAnimation(scene);
	ProcessMaterial(scene, vMaterialIndices);

	return true;
}

void MModelConverter::ProcessNode(aiNode *pNode, const aiScene *pScene, std::vector<uint32_t>& vMaterialIndices, const Matrix4& matParentRotation)
{
	Matrix4 matRotation;
	CopyMatrix4(&matRotation, &pNode->mTransformation);
	//TODO 这里只要旋转矩阵
	matRotation = matParentRotation * matRotation.GetRotatePart();
	//matRotation = matParentRotation * matRotation;

	matRotation.m[0][3] = 0;
	matRotation.m[1][3] = 0;
	matRotation.m[2][3] = 0;
	matRotation = matParentRotation * matRotation;

	for (uint32_t i = 0; i < pNode->mNumMeshes; ++i)
	{
		aiMesh* pMesh = pScene->mMeshes[pNode->mMeshes[i]];

		if (pMesh->HasBones())
		{
			MMesh<MVertexWithBones>* pMMesh = new MMesh<MVertexWithBones>();
			ProcessMeshVertices(pMesh, pScene, pMMesh);
			ProcessMeshIndices(pMesh, pScene, pMMesh);
			BindVertexAndBones(pMesh, pScene, pMMesh);
			vMaterialIndices.push_back(pMesh->mMaterialIndex);

			MMeshResource* pMeshData = m_pEngine->GetResourceManager()->CreateResource<MMeshResource>();
			pMeshData->m_strName = pNode->mName.C_Str();
			pMeshData->m_pMesh = pMMesh;
			pMeshData->m_eVertexType = MMeshResource::Skeleton;
			pMeshData->m_matRotationMatrix = matRotation;
			pMeshData->m_SkeletonKeeper.SetResource(m_pSkeleton);
			pMeshData->ResetBounds();

			pMeshData->AddRef();
			m_vMeshes.push_back(pMeshData);
		}
		else
		{
			MMesh<MVertex>* pMMesh = new MMesh<MVertex>();
			ProcessMeshVertices(pMesh, pScene, pMMesh);
			ProcessMeshIndices(pMesh, pScene, pMMesh);
			vMaterialIndices.push_back(pMesh->mMaterialIndex);

			MMeshResource* pMeshData = m_pEngine->GetResourceManager()->CreateResource<MMeshResource>();
			pMeshData->m_strName = pNode->mName.C_Str();
			pMeshData->m_pMesh = pMMesh;
			pMeshData->m_eVertexType = MMeshResource::Normal;
			pMeshData->m_matRotationMatrix = matRotation;
			pMeshData->m_SkeletonKeeper.SetResource(nullptr);

			pMeshData->ResetBounds();

			pMeshData->AddRef();
			m_vMeshes.push_back(pMeshData);
		}

	}

	for (uint32_t i = 0; i < pNode->mNumChildren; ++i)
	{
		ProcessNode(pNode->mChildren[i], pScene, vMaterialIndices, matRotation);
	}
}

void MModelConverter::ProcessMeshVertices(aiMesh* pMesh, const aiScene* pScene, MMesh<MVertex>* pMMesh)
{
	pMMesh->CreateVertices(pMesh->mNumVertices);
	for (uint32_t i = 0; i < pMesh->mNumVertices; ++i)
	{
		MVertex& vertex = pMMesh->GetVertices()[i];
		vertex.position.x = pMesh->mVertices[i].x;
		vertex.position.y = pMesh->mVertices[i].y;
		vertex.position.z = pMesh->mVertices[i].z;

	//	vertex.position = matRotation * vertex.position;

		if (pMesh->mNormals)
		{
			vertex.normal.x = pMesh->mNormals[i].x;
			vertex.normal.y = pMesh->mNormals[i].y;
			vertex.normal.z = pMesh->mNormals[i].z;

	//		vertex.normal = matRotation * vertex.normal;
		}
		if (pMesh->mTextureCoords)
		{
			if (pMesh->mTextureCoords[0])
			{
				vertex.texCoords.x = pMesh->mTextureCoords[0][i].x;
				vertex.texCoords.y = pMesh->mTextureCoords[0][i].y;
			}
		}

		if (pMesh->mTangents)
		{
			vertex.tangent.x = pMesh->mTangents[i].x;
			vertex.tangent.y = pMesh->mTangents[i].y;
			vertex.tangent.z = pMesh->mTangents[i].z;

	//		vertex.tangent = matRotation * vertex.tangent;
		}
		if (pMesh->mBitangents)
		{
			vertex.bitangent.x = pMesh->mBitangents[i].x;
			vertex.bitangent.y = pMesh->mBitangents[i].y;
			vertex.bitangent.z = pMesh->mBitangents[i].z;

	//		vertex.bitangent = matRotation * vertex.bitangent;
		}
	}
}

void MModelConverter::ProcessMeshVertices(aiMesh* pMesh, const aiScene* pScene, MMesh<MVertexWithBones>* pMMesh)
{
	pMMesh->CreateVertices(pMesh->mNumVertices);
	for (uint32_t i = 0; i < pMesh->mNumVertices; ++i)
	{
		MVertexWithBones& vertex = pMMesh->GetVertices()[i];
		vertex.position.x = pMesh->mVertices[i].x;
		vertex.position.y = pMesh->mVertices[i].y;
		vertex.position.z = pMesh->mVertices[i].z;

	//	vertex.position = matRotation * vertex.position;

		if (pMesh->mNormals)
		{
			vertex.normal.x = pMesh->mNormals[i].x;
			vertex.normal.y = pMesh->mNormals[i].y;
			vertex.normal.z = pMesh->mNormals[i].z;

	//		vertex.normal = matRotation * vertex.normal;
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

	//		vertex.tangent = matRotation * vertex.tangent;
		}
		if (pMesh->mBitangents)
		{
			vertex.bitangent.x = pMesh->mBitangents[i].x;
			vertex.bitangent.y = pMesh->mBitangents[i].y;
			vertex.bitangent.z = pMesh->mBitangents[i].z;

	//		vertex.bitangent = matRotation * vertex.bitangent;
		}
	}
}

void MModelConverter::ProcessMeshIndices(aiMesh* pMesh, const aiScene* pScene, MIMesh* pMMesh)
{
	// TODO 写死3不安全，多个顶点组成一个面的模型会有危险。
	pMMesh->CreateIndices(pMesh->mNumFaces, 3);

	for (uint32_t i = 0; i < pMesh->mNumFaces; ++i)
	{
		const aiFace& face = pMesh->mFaces[i];

		for (uint32_t j = 0; j < face.mNumIndices; ++j)
		{
			pMMesh->GetIndices()[i * 3 + j] = face.mIndices[j];
		}
	}
}

void MModelConverter::BindVertexAndBones(aiMesh* pMesh, const aiScene* pScene, MMesh<MVertexWithBones>* pMMesh)
{

	for (uint32_t i = 0; i < pMesh->mNumBones; ++i)
	{
		if (aiBone* pBone = pMesh->mBones[i])
		{
			MString strBoneName(pBone->mName.data); 
			if (const MBone* pMBone = m_pSkeleton->FindBoneByName(strBoneName))
			{
				for (uint32_t wgtIndex = 0; wgtIndex < pBone->mNumWeights; ++wgtIndex)
				{
					aiVertexWeight wgt = pBone->mWeights[wgtIndex];
					MVertexWithBones& vertex = pMMesh->GetVertices()[wgt.mVertexId];

					for (uint32_t boneIndex = 0; boneIndex < MBONES_PER_VERTEX; ++boneIndex)
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

void MModelConverter::RecordBones(aiNode* pNode, const aiScene* pScene)
{
	for (uint32_t i = 0; i < pNode->mNumMeshes; ++i)
	{
		aiMesh* pMesh = pScene->mMeshes[pNode->mMeshes[i]];
		if (pMesh->HasBones())
		{
			for (uint32_t i = 0; i < pMesh->mNumBones; ++i)
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

	for (uint32_t i = 0; i < pNode->mNumChildren; ++i)
	{
		RecordBones(pNode->mChildren[i], pScene);
	}
}

void MModelConverter::ProcessBones(const aiScene* pScene)
{
	RecordBones(pScene->mRootNode, pScene);
	BindBones(pScene->mRootNode, pScene);

	if (m_pSkeleton->GetAllBones().empty())
	{
		m_pSkeleton->SubRef();
		m_pSkeleton = nullptr;
	}
	else
	{
		m_pSkeleton->SortByDeep();
	}
}

void MModelConverter::BindBones(aiNode* pNode, const aiScene* pScene, MBone* pParent/* = nullptr*/)
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
			pMBone->unParentIndex = M_INVALID_INDEX;
		}

		CopyMatrix4(&pMBone->m_matTransform, &pNode->mTransformation);
	}

	for (uint32_t i = 0; i < pNode->mNumChildren; ++i)
	{
		BindBones(pNode->mChildren[i], pScene, pMBone);
	}
}

void MModelConverter::ProcessAnimation(const aiScene* pScene)
{
	if (!m_pSkeleton)
		return;

	for (uint32_t i = 0; i < pScene->mNumAnimations; ++i)
	{
		aiAnimation* pAnimation = pScene->mAnimations[i];

		MSkeletalAnimationResource* pMAnimation = m_pEngine->GetResourceManager()->CreateResource<MSkeletalAnimationResource>();
		pMAnimation->AddRef();

		m_vSkeletalAnimation.push_back(pMAnimation);

		pMAnimation->m_Skeleton = m_pSkeleton;
//		pMAnimation->m_vSkeletalAnimNodes.resize(pAnimation->mNumChannels);// init by nullptr
		pMAnimation->m_vSkeletalAnimNodes.resize(m_pSkeleton->GetAllBones().size());

		pMAnimation->m_unIndex = i;
		pMAnimation->m_strName = pAnimation->mName.C_Str();
		pMAnimation->m_fTicksDuration = pAnimation->mDuration;
		if(pAnimation->mTicksPerSecond > 0.0f)
			pMAnimation->m_fTicksPerSecond = pAnimation->mTicksPerSecond;

		for (uint32_t chanIndex = 0; chanIndex < pAnimation->mNumChannels; ++chanIndex)
		{
			aiNodeAnim* pNodeAnim = pAnimation->mChannels[chanIndex];

			if (MBone* pBone = m_pSkeleton->FindBoneByName(pNodeAnim->mNodeName.C_Str()))
			{
				MSkeletalAnimNode& mAnimNode = pMAnimation->m_vSkeletalAnimNodes[pBone->unIndex];

				mAnimNode.m_unPositionKeysNum = pNodeAnim->mNumPositionKeys;
				mAnimNode.m_unRotationKeysNum = pNodeAnim->mNumRotationKeys;
				mAnimNode.m_unScalingKeysNum = pNodeAnim->mNumScalingKeys;
				
				if (mAnimNode.m_unPositionKeysNum > 0)
				{
					mAnimNode.m_vPositionKeys = new MSkeletalAnimNode::MAnimNodeKey<Vector3>[mAnimNode.m_unPositionKeysNum];
					for (unsigned keyIndex = 0; keyIndex < mAnimNode.m_unPositionKeysNum; ++keyIndex)
					{
						MSkeletalAnimNode::MAnimNodeKey<Vector3>& dkey = mAnimNode.m_vPositionKeys[keyIndex];
						const aiVectorKey& skey = pNodeAnim->mPositionKeys[keyIndex];
						dkey.mTime = skey.mTime;
						dkey.mValue.x = skey.mValue.x;
						dkey.mValue.y = skey.mValue.y;
						dkey.mValue.z = skey.mValue.z;
					}
				}
				if (mAnimNode.m_unRotationKeysNum > 0)
				{
					mAnimNode.m_vRotationKeys = new MSkeletalAnimNode::MAnimNodeKey<Quaternion>[mAnimNode.m_unRotationKeysNum];
					for (unsigned keyIndex = 0; keyIndex < mAnimNode.m_unRotationKeysNum; ++keyIndex)
					{
						MSkeletalAnimNode::MAnimNodeKey<Quaternion>& dkey = mAnimNode.m_vRotationKeys[keyIndex];
						const aiQuatKey& skey = pNodeAnim->mRotationKeys[keyIndex];
						dkey.mTime = skey.mTime;
						dkey.mValue.w = skey.mValue.w;
						dkey.mValue.x = skey.mValue.x;
						dkey.mValue.y = skey.mValue.y;
						dkey.mValue.z = skey.mValue.z;
					}
				}
				if (mAnimNode.m_unScalingKeysNum > 0)
				{
					mAnimNode.m_vScalingKeys = new MSkeletalAnimNode::MAnimNodeKey<Vector3>[mAnimNode.m_unScalingKeysNum];
					for (unsigned keyIndex = 0; keyIndex < mAnimNode.m_unScalingKeysNum; ++keyIndex)
					{
						MSkeletalAnimNode::MAnimNodeKey<Vector3>& dkey = mAnimNode.m_vScalingKeys[keyIndex];
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

void MModelConverter::ProcessMaterial(const aiScene* pScene, std::vector<uint32_t>& vMaterialIndices)
{
	aiString strAmbient("$clr.ambient"), strDiffuse("$clr.diffuse"), strSpecular("$clr.specular"), strShininess("$mat.shininess");

	MMaterialResource* pStaticMeshMaterial = m_pEngine->GetResourceManager()->LoadVirtualResource<MMaterialResource>(DEFAULT_MATERIAL_STATIC);
	MMaterialResource* pSkinnedMeshMaterial = m_pEngine->GetResourceManager()->LoadVirtualResource<MMaterialResource>(DEFAULT_MATERIAL_SKINNED);

	for (uint32_t i = 0; i < m_vMeshes.size(); ++i)
	{
		MMeshResource* pMeshData = m_vMeshes[i];
		MMaterialResource* pMaterial = m_pEngine->GetResourceManager()->CreateResource<MMaterialResource>();
		pMeshData->m_MaterialKeeper.SetResource(pMaterial);

		if (pMeshData->GetMeshVertexType() == MMeshResource::Normal)
			pMaterial->CopyFrom(pStaticMeshMaterial);
		else if(pMeshData->GetMeshVertexType() == MMeshResource::Skeleton)
			pMaterial->CopyFrom(pSkinnedMeshMaterial);

		uint32_t unMaterialIndex = vMaterialIndices[i];

		if(unMaterialIndex >= pScene->mNumMaterials)
			continue;

		aiMaterial* pAiMaterial = pScene->mMaterials[unMaterialIndex];

		Vector3 v3Ambient(1.0f, 1.0f, 1.0f), v3Diffuse(1.0f, 1.0f, 1.0f), v3Specular(1.0f, 1.0f, 1.0f);
		float fShininess = 32.0f;
		MString strTexPath;

		for (int n = 0; n < pAiMaterial->mNumProperties; ++n)
		{
			aiMaterialProperty* prop = pAiMaterial->mProperties[n];
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
		MStruct* pMaterialStruct = nullptr;
		for (MShaderConstantParam* pParam : *pMaterial->GetShaderParams())
		{
			if (MStruct* pStruct = pParam->var.GetStruct())
			{
				if (MStruct* pMat = pStruct->FindMember("U_mat")->GetStruct())
				{
					pMaterialStruct = pMat;

					pMat->SetMember("f3Ambient", v3Ambient);
					pMat->SetMember("f3Diffuse", v3Diffuse);
					pMat->SetMember("f3Specular", v3Specular);
					pMat->SetMember("fShininess", fShininess);
					pMat->SetMember("bUseNormalTex", false);
					pMat->SetMember("fAlphaFactor", 1.0f);
				}
			}

			continue;
		}

		MString strResourceFolder = MResource::GetFolder(m_strResourcePath);

		aiString strDiffuseTextureFile, strNormalTextureFile;
		pAiMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &strDiffuseTextureFile);
		pAiMaterial->GetTexture(aiTextureType_NORMALS, 0, &strNormalTextureFile);
		MString strDiffuseFileName = MResource::GetFileName(MString(strDiffuseTextureFile.C_Str()));
		MString strNormalFileName = MResource::GetFileName(MString(strNormalTextureFile.C_Str()));

		MResource* pDiffuseTexRes = nullptr;
		MResource* pNormalMapRes = nullptr;

		if (!strDiffuseFileName.empty())
			pDiffuseTexRes = m_pEngine->GetResourceManager()->LoadResource(strResourceFolder + "/tex/" + strDiffuseFileName);
		else
			pDiffuseTexRes = m_pEngine->GetResourceManager()->LoadVirtualResource<MTextureResource>(DEFAULT_TEXTURE_WHITE);

		if (!strNormalFileName.empty())
		{
			pNormalMapRes = m_pEngine->GetResourceManager()->LoadResource(strResourceFolder + "/tex/" + strNormalFileName);
			if (pMaterialStruct)
				pMaterialStruct->SetMember("bUseNormalTex", true);

		}
		else
			pNormalMapRes = m_pEngine->GetResourceManager()->LoadVirtualResource<MTextureResource>(DEFAULT_TEXTURE_NORMALMAP);

		pMaterial->SetTexutreParam(SHADER_PARAM_NAME_DIFFUSE, pDiffuseTexRes);
		pMaterial->SetTexutreParam(SHADER_PARAM_NAME_NORMAL, pNormalMapRes);
	}
}
