#include "MModelConverter.h"

#include "MLogManager.h"
#include "MMesh.h"
#include "MResourceManager.h"
#include "MVertex.h"
#include "MMeshResource.h"

#include "MIDevice.h"
#include "MEngine.h"

#include "Model/MModelInstance.h"
#include "Model/MModelResource.h"
#include "Model/MStaticMeshInstance.h"
#include "Model/MSkeletonResource.h"
#include "Texture/MTextureResource.h"
#include "Material/MMaterialResource.h"
#include "Model/MSkeletalAnimationResource.h"
#include "Node/MNodeResource.h"

#include "MCamera.h"
#include "Light/MSpotLight.h"
#include "Light/MPointLight.h"
#include "Light/MDirectionalLight.h"

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

MColor GetColor(const aiColor3D& color)
{
	return MColor(color.r, color.g, color.b);
}

Vector3 GetVector3(const aiVector3D& val)
{
	return Vector3(val.x, val.y, val.z);
}

MModelConverter::MModelConverter(MEngine* pEngine)
: m_pEngine(pEngine)
, m_strResourcePath()
, m_vMeshes()
, m_pSkeleton(nullptr)
, m_pModelInstance(nullptr)
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

	for (int i = 0; i < m_vMaterials.size(); ++i)
	{
		if (m_vMaterials[i])
		{
			MString strMaterialFileName = strPath + "material_" + MStringHelper::ToString(i);
			m_pEngine->GetResourceManager()->MoveTo(m_vMaterials[i], strMaterialFileName + "." + SUFFIX_MATERIAL);
			m_vMaterials[i]->Save();
		}
	}

	for (int i = 0; i < m_vMeshes.size(); ++i)
	{
		MMeshResource* pMeshResource = m_vMeshes[i];
		MString strMeshFileName = strPath + pMeshResource->GetMeshName() + "_" + MStringHelper::ToString(i);

		
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

	MNodeResource* pNodeResource = m_pEngine->GetResourceManager()->CreateResource<MNodeResource>();
	pNodeResource->AddRef();

	m_pEngine->GetResourceManager()->MoveTo(pNodeResource, strPath + strOutputName + "." + SUFFIX_NODE);
	pNodeResource->SaveByNode(m_pModelInstance);

	pNodeResource->SubRef();
	pNodeResource = nullptr;

	return true;
}

bool MModelConverter::Load(const MString& strResourcePath)
{
	m_pSkeleton = m_pEngine->GetResourceManager()->CreateResource<MSkeletonResource>();
	m_pSkeleton->AddRef();

	m_pModelInstance = m_pEngine->GetObjectManager()->CreateObject<MModelInstance>();

	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(strResourcePath, aiProcess_JoinIdenticalVertices | aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace | aiProcess_FixInfacingNormals | aiProcess_ConvertToLeftHanded);
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
	{
		MLogManager::GetInstance()->Error("ERROR::ASSIMP:: %s", importer.GetErrorString());
		return false;
	}

	//Bones
	ProcessBones(scene);
	m_pModelInstance->SetSkeletonTemplate(m_pSkeleton);

	//Meshes
	ProcessNode(scene->mRootNode, scene);

	//Animation
	ProcessAnimation(scene);

	//Lights
	ProcessLights(scene);

	//Cameras
	ProcessCameras(scene);

	return true;
}

void MModelConverter::ProcessNode(aiNode* pNode, const aiScene *pScene)
{
	for (uint32_t i = 0; i < pNode->mNumMeshes; ++i)
	{
		aiMesh* pChildMesh = pScene->mMeshes[pNode->mMeshes[i]];

		MMeshResource* pChildMeshResource = m_pEngine->GetResourceManager()->CreateResource<MMeshResource>();
		pChildMeshResource->AddRef();

		if (m_pSkeleton)
		{
			MMesh<MVertexWithBones>* pMBonesMesh = new MMesh<MVertexWithBones>();
			ProcessMeshVertices(pChildMesh, pScene, pMBonesMesh);
			ProcessMeshIndices(pChildMesh, pScene, pMBonesMesh);
			BindVertexAndBones(pChildMesh, pScene, pMBonesMesh);
			pChildMeshResource->m_pMesh = pMBonesMesh;
			pChildMeshResource->m_eVertexType = MMeshResource::Skeleton;
		}
		else
		{
			MMesh<MVertex>* pMStaticMesh = new MMesh<MVertex>();
			ProcessMeshVertices(pChildMesh, pScene, pMStaticMesh);
			ProcessMeshIndices(pChildMesh, pScene, pMStaticMesh);
			pChildMeshResource->m_eVertexType = MMeshResource::Skeleton;

			pChildMeshResource->m_pMesh = pMStaticMesh;
			pChildMeshResource->m_eVertexType = MMeshResource::Normal;
		}

		pChildMeshResource->m_strName = pNode->mName.C_Str();
		pChildMeshResource->m_SkeletonKeeper.SetResource(m_pSkeleton);
		pChildMeshResource->m_MaterialKeeper.SetResource(GetMaterial(pScene, pChildMesh->mMaterialIndex));
		pChildMeshResource->ResetBounds();
		m_vMeshes.push_back(pChildMeshResource);


		MStaticMeshInstance* pChildMeshNode = m_pEngine->GetObjectManager()->CreateObject<MStaticMeshInstance>();
		pChildMeshNode->SetName(pChildMesh->mName.C_Str());
		pChildMeshNode->SetAttachedModelInstance(m_pModelInstance);
		pChildMeshNode->Load(pChildMeshResource);

		GetMNodeFromNode(pScene, pNode)->AddNodeImpl(pChildMeshNode, MNode::EProtected);
	}

	for (uint32_t i = 0; i < pNode->mNumChildren; ++i)
	{
		aiNode* pChild = pNode->mChildren[i];
		ProcessNode(pChild, pScene);
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


		if (pMesh->mNormals)
		{
			vertex.normal.x = pMesh->mNormals[i].x;
			vertex.normal.y = pMesh->mNormals[i].y;
			vertex.normal.z = pMesh->mNormals[i].z;

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

		}
		if (pMesh->mBitangents)
		{
			vertex.bitangent.x = pMesh->mBitangents[i].x;
			vertex.bitangent.y = pMesh->mBitangents[i].y;
			vertex.bitangent.z = pMesh->mBitangents[i].z;
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

	for (uint32_t i = 0; i < pMMesh->GetVerticesLength(); ++i)
	{
		MVertexWithBones& vertex = pMMesh->GetVertices()[i];

		float fLength = 0.0f;
		for (uint32_t n = 0; n < MBONES_PER_VERTEX; ++n)
			fLength += vertex.bonesWeight[n];

		if (fLength > 0.0f)
		{
			for (uint32_t n = 0; n < MBONES_PER_VERTEX; ++n)
				vertex.bonesWeight[n] *= (1.0f / fLength);
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

void MModelConverter::ProcessLights(const aiScene* pScene)
{
	for (uint32_t i = 0; i < pScene->mNumLights; ++i)
	{
		aiLight* pLight = pScene->mLights[i];

		MString strName = pLight->mName.C_Str();

		MILight* pMLight = nullptr;
		switch (pLight->mType)
		{
		case aiLightSourceType::aiLightSource_POINT:
		{
			MPointLight* pPointLight = m_pEngine->GetObjectManager()->CreateObject<MPointLight>();
			pPointLight->SetDiffuseColor(GetColor(pLight->mColorDiffuse));
			pPointLight->SetSpecularColor(GetColor(pLight->mColorSpecular));
			pPointLight->SetPosition(GetVector3(pLight->mPosition));

			pMLight = pPointLight;
		}
		break;

		case aiLightSourceType::aiLightSource_DIRECTIONAL:
		{
			MDirectionalLight* pDirLight = m_pEngine->GetObjectManager()->CreateObject<MDirectionalLight>();
			pDirLight->SetDiffuseColor(GetColor(pLight->mColorDiffuse));
			pDirLight->SetSpecularColor(GetColor(pLight->mColorSpecular));
			pDirLight->LookAt(GetVector3(pLight->mDirection), GetVector3((pLight->mUp)));
			pDirLight->SetPosition(GetVector3(pLight->mPosition));

			pMLight = pDirLight;
		}
		break;
		case aiLightSourceType::aiLightSource_SPOT:
		{
			MSpotLight* pSpotLight = m_pEngine->GetObjectManager()->CreateObject<MSpotLight>();
			pSpotLight->SetDiffuseColor(GetColor(pLight->mColorDiffuse));
			pSpotLight->SetSpecularColor(GetColor(pLight->mColorSpecular));
			pSpotLight->LookAt(GetVector3(pLight->mDirection), GetVector3((pLight->mUp)));
			pSpotLight->SetPosition(GetVector3(pLight->mPosition));

			pSpotLight->SetInnerCutOff(pLight->mAngleInnerCone);
			pSpotLight->SetOuterCutOff(pLight->mAngleOuterCone);

			pMLight = pSpotLight;
		}
		break;

		default:
			break;
		}
	}
}

void MModelConverter::ProcessCameras(const aiScene* pScene)
{
	for (uint32_t i = 0; i < pScene->mNumCameras; ++i)
	{
		aiCamera* pCamera = pScene->mCameras[i];

		MCamera* pMCamera = m_pEngine->GetObjectManager()->CreateObject<MCamera>();

		pMCamera->SetCameraType(MCamera::MECameraType::EPerspective);
		pMCamera->SetZNear(pCamera->mClipPlaneNear);
		pMCamera->SetZFar(pCamera->mClipPlaneFar);
		pMCamera->SetFov(pCamera->mHorizontalFOV);

		pMCamera->SetPosition(GetVector3(pCamera->mPosition));
		pMCamera->LookAt(GetVector3(pCamera->mLookAt), GetVector3(pCamera->mUp));
		

		GetMNodeFromNode(pScene, pScene->mRootNode)->AddNodeImpl(pMCamera, MNode::EProtected);
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

void MModelConverter::ProcessMaterial(const aiScene* pScene, const uint32_t& nMaterialIdx)
{
	aiString strAmbient("$clr.ambient"), strDiffuse("$clr.diffuse"), strSpecular("$clr.specular"), strShininess("$mat.shininess");

	MMaterialResource* pMaterial = m_pEngine->GetResourceManager()->CreateResource<MMaterialResource>();
	
	if (m_pSkeleton)
	{
		MMaterialResource* pSkinnedMeshMaterial = m_pEngine->GetResourceManager()->LoadVirtualResource<MMaterialResource>(DEFAULT_MATERIAL_MODEL_SKELETON_MESH);
		pMaterial->CopyFrom(pSkinnedMeshMaterial);
	}
	else
	{
		MMaterialResource* pStaticMeshMaterial = m_pEngine->GetResourceManager()->LoadVirtualResource<MMaterialResource>(DEFAULT_MATERIAL_MODEL_STATIC_MESH);
		pMaterial->CopyFrom(pStaticMeshMaterial);
	}

	if(nMaterialIdx >= pScene->mNumMaterials)
		return;

	aiMaterial* pAiMaterial = pScene->mMaterials[nMaterialIdx];

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

	if (0.0f == fShininess)
		fShininess = 32.0f;

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
	

	m_vMaterials[nMaterialIdx] = pMaterial;
}

M3DNode* MModelConverter::GetMNodeFromNode(const aiScene* pScene, aiNode* pNode)
{
	if (pScene->mRootNode == pNode)
		return m_pModelInstance;

	if (M3DNode* pMNode = m_tNodeMaps[pNode])
		return pMNode;

	Matrix4 matTransform;
	CopyMatrix4(&matTransform, &pNode->mTransformation);

	M3DNode* pMParent = GetMNodeFromNode(pScene, pNode->mParent);

	M3DNode* pMNode = m_pEngine->GetObjectManager()->CreateObject<M3DNode>();
	pMNode->SetName(pNode->mName.C_Str());
	pMNode->SetTransform(MTransform(matTransform));
	pMParent->AddNodeImpl(pMNode, MNode::EProtected);
	m_tNodeMaps[pNode] = pMNode;

	return pMNode;
}

MMaterial* MModelConverter::GetMaterial(const aiScene* pScene, const uint32_t& nMaterialIdx)
{
	if (m_vMaterials.size() <= nMaterialIdx)
		m_vMaterials.resize(nMaterialIdx + 1);

	if (!m_vMaterials[nMaterialIdx])
		ProcessMaterial(pScene, nMaterialIdx);

	return m_vMaterials[nMaterialIdx];
}
