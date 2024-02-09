#include "Model/MModelConverter.h"
#include "MRenderModule.h"

#include "Render/MMesh.h"
#include "Utility/MTimer.h"
#include "Scene/MScene.h"
#include "Render/MVertex.h"
#include "Utility/MLogger.h"
#include "Engine/MEngine.h"
#include "Render/MIDevice.h"

#include "Resource/MMeshResource.h"
#include "Resource/MEntityResource.h"
#include "Resource/MTextureResource.h"
#include "Resource/MMaterialResource.h"
#include "Resource/MSkeletonResource.h"
#include "Resource/MSkeletalAnimationResource.h"
#include "Resource/MTextureResourceUtil.h"

#include "Utility/MBounds.h"
#include "Model/MSkeleton.h"
#include "Model/MSkeletalAnimation.h"

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "assimp/material.h"

#include "Utility/MFileHelper.h"


#include "Component/MComponent.h"
#include "Component/MModelComponent.h"
#include "Component/MSceneComponent.h"
#include "Component/MCameraComponent.h"
#include "Component/MSpotLightComponent.h"
#include "Component/MPointLightComponent.h"
#include "Component/MRenderMeshComponent.h"
#include "Component/MDirectionalLightComponent.h"

#include "System/MEntitySystem.h"
#include "System/MObjectSystem.h"
#include "System/MResourceSystem.h"

#include <fstream>

#include "Render/MMaterialName.h"
#include "Resource/MReadableTextureResource.h"

static const std::map<aiTextureType, MEModelTextureUsage> TextureUsageMapping = {
			{aiTextureType_DIFFUSE, MEModelTextureUsage::BaseColor},
			{aiTextureType_NORMALS, MEModelTextureUsage::Normal},

			{aiTextureType_BASE_COLOR, MEModelTextureUsage::BaseColor},
			{aiTextureType_NORMAL_CAMERA, MEModelTextureUsage::Normal},
			{aiTextureType_METALNESS, MEModelTextureUsage::Metallic},
			{aiTextureType_DIFFUSE_ROUGHNESS, MEModelTextureUsage::Roughness},
			{aiTextureType_AMBIENT_OCCLUSION, MEModelTextureUsage::AmbientOcc},
			{aiTextureType_EMISSION_COLOR, MEModelTextureUsage::Emission},
};

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

void CopyMatrix4Transposed(Matrix4* matdest, aiMatrix4x4* matsour)
{
	for (uint32_t r = 0; r < 4; ++r)
	{
		for (uint32_t c = 0; c < 4; ++c)
		{
			matdest->m[r][c] = (*matsour)[c][r];
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
	, m_pScene(nullptr)
	, m_strResourcePath()
	, m_vMeshes()
	, m_pSkeletonResource(nullptr)
	, m_pModelEntity(nullptr)
	, m_vSkeletalAnimation()
	, eMaterialType(MModelConvertMaterialType::E_Default_Forward)
{
}

MModelConverter::~MModelConverter()
{
	if (m_pSkeletonResource)
	{
		m_pSkeletonResource = nullptr;
	}

	m_vMeshes.clear();

	m_vSkeletalAnimation.clear();
}
bool MModelConverter::Convert(const MModelConvertInfo& convertInfo)
{
	MObjectSystem* pObjectSystem = GetEngine()->FindSystem<MObjectSystem>();
	MResourceSystem* pResourceSystem = GetEngine()->FindSystem<MResourceSystem>();
	MEntitySystem* pEntitySystem = GetEngine()->FindSystem<MEntitySystem>();

	m_pScene = pObjectSystem->CreateObject<MScene>();

	m_strResourcePath = convertInfo.strResourcePath;
	bImportCamera = convertInfo.bImportCamera;
	bImportLights = convertInfo.bImportLights;
	eMaterialType = convertInfo.eMaterialType;
	m_pTextureDelegate = convertInfo.pTextureDelegate;

	auto time = MTimer::GetCurTime();
	if (!Load(convertInfo.strResourcePath))
		return false;

	time = MTimer::GetCurTime() - time;

	GetEngine()->GetLogger()->Log("Load Model Time: {}", time);

	MString strPath = convertInfo.strOutputDir + "/" + convertInfo.strOutputName + "/";

	MFileHelper::MakeDir(convertInfo.strOutputDir);

	if (m_pSkeletonResource)
	{
		pResourceSystem->MoveTo(m_pSkeletonResource, strPath + convertInfo.strOutputName + ".ske");
		pResourceSystem->SaveResource(m_pSkeletonResource);
	}

	for (auto pr : m_tRawTextures)
	{
		if (pr.second)
		{
			std::shared_ptr<MTextureResource> pTextureResource = pr.second;
			MString strValidFileName = pr.first;
			pResourceSystem->MoveTo(pTextureResource, strPath + strValidFileName + ".mtex");
			pResourceSystem->SaveResource(pTextureResource);
		}
	}

	for (auto pTexture : m_tFileTextures)
	{
		if (pTexture)
		{
			MString strValidFileName = MFileHelper::GetFileName(pTexture->GetResourcePath());
			pResourceSystem->MoveTo(pTexture, strPath + strValidFileName + ".mtex");
			pResourceSystem->SaveResource(pTexture);
		}
	}

	for (size_t i = 0; i < m_vMaterials.size(); ++i)
	{
		if (m_vMaterials[i])
		{
			MString strMaterialFileName = strPath + "material_" + MStringUtil::ToString(i);
			pResourceSystem->MoveTo(m_vMaterials[i], strMaterialFileName + ".mat");
			pResourceSystem->SaveResource(m_vMaterials[i]);
		}
	}

	for (size_t i = 0; i < m_vMeshes.size(); ++i)
	{
		std::shared_ptr<MMeshResource> pMeshResource = m_vMeshes[i].second;
		MString strMeshFileName = strPath + m_vMeshes[i].first + "_" + MStringUtil::ToString(i);

		pResourceSystem->MoveTo(pMeshResource, strMeshFileName + ".mesh");
		pResourceSystem->SaveResource(pMeshResource);
	}

	for (std::shared_ptr<MSkeletalAnimationResource> pAnimResource : m_vSkeletalAnimation)
	{
		MString strValidFileName = pAnimResource->GetAnimationName();
		MFileHelper::GetValidFileName(strValidFileName);
		pResourceSystem->MoveTo(pAnimResource, strPath + strValidFileName + ".anim");
		pResourceSystem->SaveResource(pAnimResource);

	}


	auto vAllEntity = m_pScene->GetAllEntity();
	std::shared_ptr<MResource> pNodeResource = pEntitySystem->PackEntity(vAllEntity);

	pResourceSystem->MoveTo(pNodeResource, strPath + convertInfo.strOutputName + ".entity");
	pResourceSystem->SaveResource(pNodeResource);

	pNodeResource = nullptr;

	return true;
}

bool MModelConverter::Load(const MString& strResourcePath)
{
	MResourceSystem* pResourceSystem = GetEngine()->FindSystem<MResourceSystem>();

	std::vector<MString> vSearchPath = pResourceSystem->GetSearchPath();
	
	Assimp::Importer importer;
	const aiScene* scene = nullptr;
	for (MString strSearchPath : vSearchPath)
	{
		std::string strFullpath = strSearchPath + strResourcePath;
		std::ifstream ifs(strFullpath.c_str(), std::ios::binary);
		if (!ifs.good())
			continue;

		scene = importer.ReadFile(strFullpath, aiProcess_JoinIdenticalVertices | aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace | aiProcess_FixInfacingNormals | aiProcess_ConvertToLeftHanded);
	
		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
		{
			GetEngine()->GetLogger()->Error("ERROR::ASSIMP:: {}", importer.GetErrorString());
			return false;
		}
		break;
	}

	if (!scene)
		return false;
	

	m_pModelEntity = GetEntityFromNode(scene, scene->mRootNode);
	MModelComponent* pModelComponent = m_pScene->AddComponent<MModelComponent>(m_pModelEntity);

	//Textures
	ProcessTexture(scene);

	//Bones
	ProcessBones(scene);
	pModelComponent->SetSkeletonResource(m_pSkeletonResource);

	//Meshes
	ProcessNode(scene->mRootNode, scene);

	//Animation
	ProcessAnimation(scene);

	if (bImportLights)
	{
		//Lights
		ProcessLights(scene);
	}

	if (bImportCamera)
	{
		//Cameras
		ProcessCameras(scene);
	}

	return true;
}

void MModelConverter::ProcessNode(aiNode* pNode, const aiScene* pScene)
{
	MEntitySystem* pEntitySystem = GetEngine()->FindSystem<MEntitySystem>();
	MResourceSystem* pResourceSystem = GetEngine()->FindSystem<MResourceSystem>();

	for (uint32_t i = 0; i < pNode->mNumMeshes; ++i)
	{
		aiMesh* pChildMesh = pScene->mMeshes[pNode->mMeshes[i]];

		std::unique_ptr<MResourceData> pResourceData = std::make_unique<MMeshResourceData>();
		auto pMeshResourceData = static_cast<MMeshResourceData*>(pResourceData.get());

		if (m_pSkeletonResource)
		{
			MSkeleton* pSkeleton = m_pSkeletonResource->GetSkeleton();
			MMesh<MVertexWithBones>* pMBonesMesh = new MMesh<MVertexWithBones>();
			ProcessMeshVertices(pChildMesh, pMBonesMesh);
			ProcessMeshIndices(pChildMesh, pMBonesMesh);
			BindVertexAndBones(pSkeleton, pChildMesh, pMBonesMesh);
			pMeshResourceData->pMesh.reset(pMBonesMesh);
			pMeshResourceData->eVertexType = MEMeshVertexType::Skeleton;
		}
		else
		{
			MMesh<MVertex>* pMStaticMesh = new MMesh<MVertex>();
			ProcessMeshVertices(pChildMesh, pMStaticMesh);
			ProcessMeshIndices(pChildMesh, pMStaticMesh);
			pMeshResourceData->pMesh.reset(pMStaticMesh);
			pMeshResourceData->eVertexType = MEMeshVertexType::Normal;
		}

		std::shared_ptr<MMeshResource> pChildMeshResource = pResourceSystem->CreateResource<MMeshResource>();
		pChildMeshResource->Load(std::move(pResourceData));

		MString strMeshName = pChildMesh->mName.C_Str();
		pChildMeshResource->ResetBounds();
		m_vMeshes.push_back({ strMeshName, pChildMeshResource });


		MEntity* pChildEntity = m_pScene->CreateEntity();
		pChildEntity->SetName(pChildMesh->mName.C_Str());
		pChildEntity->RegisterComponent<MSceneComponent>();

		MRenderMeshComponent* pMeshComponent = pChildEntity->RegisterComponent<MRenderMeshComponent>();
		pMeshComponent->Load(pChildMeshResource);
		pMeshComponent->SetMaterial(GetMaterial(pScene, pChildMesh->mMaterialIndex));

		pEntitySystem->AddChild(GetEntityFromNode(pScene, pNode), pChildEntity);
	}

	for (uint32_t i = 0; i < pNode->mNumChildren; ++i)
	{
		aiNode* pChild = pNode->mChildren[i];
		ProcessNode(pChild, pScene);
	}
}

void MModelConverter::ProcessMeshVertices(aiMesh* pMesh, MMesh<MVertex>* pMMesh)
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

		//if (pMesh->mTextureCoords)
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

void MModelConverter::ProcessMeshVertices(aiMesh* pMesh, MMesh<MVertexWithBones>* pMMesh)
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

		//if (pMesh->mTextureCoords)
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

void MModelConverter::ProcessMeshIndices(aiMesh* pMesh, MIMesh* pMMesh)
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

void MModelConverter::BindVertexAndBones(MSkeleton* pSkeleton, aiMesh* pMesh, MMesh<MVertexWithBones>* pMMesh)
{

	for (uint32_t i = 0; i < pMesh->mNumBones; ++i)
	{
		if (aiBone* pBone = pMesh->mBones[i])
		{
			MString strBoneName(pBone->mName.data);
			if (const MBone* pMBone = pSkeleton->FindBoneByName(strBoneName))
			{
				for (uint32_t wgtIndex = 0; wgtIndex < pBone->mNumWeights; ++wgtIndex)
				{
					aiVertexWeight wgt = pBone->mWeights[wgtIndex];
					MVertexWithBones& vertex = pMMesh->GetVertices()[wgt.mVertexId];

					for (uint32_t boneIndex = 0; boneIndex < MRenderGlobal::BONES_PER_VERTEX; ++boneIndex)
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

	for (uint32_t i = 0; i < pMMesh->GetVerticesNum(); ++i)
	{
		MVertexWithBones& vertex = pMMesh->GetVertices()[i];

		float fLength = 0.0f;
		for (uint32_t n = 0; n < MRenderGlobal::BONES_PER_VERTEX; ++n)
			fLength += vertex.bonesWeight[n];

		if (fLength > 0.0f)
		{
			for (uint32_t n = 0; n < MRenderGlobal::BONES_PER_VERTEX; ++n)
				vertex.bonesWeight[n] *= (1.0f / fLength);
		}

	}
}

void MModelConverter::RecordBones(MSkeleton* pSkeleton, aiNode* pNode, const aiScene* pScene)
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
					MBone* pMBone = pSkeleton->FindBoneByName(strBoneName);
					if (nullptr == pMBone)
					{
						pMBone = pSkeleton->AppendBone(strBoneName);
						CopyMatrix4Transposed(&pMBone->m_matOffsetMatrix, &pBone->mOffsetMatrix);
					}
				}
			}
		}
	}

	for (uint32_t i = 0; i < pNode->mNumChildren; ++i)
	{
		RecordBones(pSkeleton, pNode->mChildren[i], pScene);
	}
}

void MModelConverter::ProcessBones(const aiScene* pScene)
{
	std::unique_ptr<MResourceData> pResourceData = std::make_unique<MSkeletonResourceData>();

	MSkeletonResourceData* pSkeletonData = static_cast<MSkeletonResourceData*>(pResourceData.get());

	RecordBones(&pSkeletonData->skeleton, pScene->mRootNode, pScene);
	BindBones(&pSkeletonData->skeleton, pScene->mRootNode, pScene);

	if (pSkeletonData->skeleton.GetAllBones().empty())
	{
		return;
	}

	MResourceSystem* pResourceSystem = GetEngine()->FindSystem<MResourceSystem>();
	m_pSkeletonResource = pResourceSystem->CreateResource<MSkeletonResource>();

	pSkeletonData->skeleton.SortByDeep();
	m_pSkeletonResource->Load(std::move(pResourceData));
}

void MModelConverter::BindBones(MSkeleton* pSkeleton, aiNode* pNode, const aiScene* pScene, MBone* pParent/* = nullptr*/)
{
	MBone* pMBone = pSkeleton->FindBoneByName(pNode->mName.data);
	if (pMBone)
	{
		if (pParent)
		{
			pMBone->unParentIndex = pParent->unIndex;
			pParent->vChildrenIndices.push_back(pMBone->unIndex);
		}
		else
		{
			pMBone->unParentIndex = MGlobal::M_INVALID_INDEX;
		}

		CopyMatrix4(&pMBone->m_matTransform, &pNode->mTransformation);
	}

	for (uint32_t i = 0; i < pNode->mNumChildren; ++i)
	{
		BindBones(pSkeleton, pNode->mChildren[i], pScene, pMBone);
	}
}

void MModelConverter::ProcessLights(const aiScene* pScene)
{
	for (uint32_t i = 0; i < pScene->mNumLights; ++i)
	{
		aiLight* pLight = pScene->mLights[i];

		MString strName = pLight->mName.C_Str();

		MEntity* pLightEntity = m_pScene->CreateEntity();
		pLightEntity->SetName(strName);
		switch (pLight->mType)
		{
		case aiLightSourceType::aiLightSource_POINT:
		{
			MSceneComponent* pSceneComponent = m_pScene->AddComponent<MSceneComponent>(pLightEntity);
			MPointLightComponent* pLightComponent = m_pScene->AddComponent<MPointLightComponent>(pLightEntity);

			pSceneComponent->SetPosition(GetVector3(pLight->mPosition));
			pLightComponent->SetColor(GetColor(pLight->mColorDiffuse));

		}
		break;

		case aiLightSourceType::aiLightSource_DIRECTIONAL:
		{
			MSceneComponent* pSceneComponent = m_pScene->AddComponent<MSceneComponent>(pLightEntity);
			MDirectionalLightComponent* pLightComponent = m_pScene->AddComponent<MDirectionalLightComponent>(pLightEntity);

			pSceneComponent->SetPosition(GetVector3(pLight->mPosition));
			pSceneComponent->LookAt(GetVector3(pLight->mDirection), GetVector3((pLight->mUp)));
			pLightComponent->SetColor(GetColor(pLight->mColorDiffuse));

		}
		break;
		case aiLightSourceType::aiLightSource_SPOT:
		{
			MSceneComponent* pSceneComponent = m_pScene->AddComponent<MSceneComponent>(pLightEntity);
			MSpotLightComponent* pLightComponent = m_pScene->AddComponent<MSpotLightComponent>(pLightEntity);

			pSceneComponent->SetPosition(GetVector3(pLight->mPosition));
			pSceneComponent->LookAt(GetVector3(pLight->mDirection), GetVector3((pLight->mUp)));
			pLightComponent->SetColor(GetColor(pLight->mColorDiffuse));

			pLightComponent->SetInnerCutOff(pLight->mAngleInnerCone);
			pLightComponent->SetOuterCutOff(pLight->mAngleOuterCone);

		}
		break;

		default:
			break;
		}
	}
}

void MModelConverter::ProcessCameras(const aiScene* pScene)
{
	MEntitySystem* pEntitySystem = GetEngine()->FindSystem<MEntitySystem>();

	for (uint32_t i = 0; i < pScene->mNumCameras; ++i)
	{
		aiCamera* pCamera = pScene->mCameras[i];
		MString strName = pCamera->mName.C_Str();

		MEntity* pCameraEntity = m_pScene->CreateEntity();
		pCameraEntity->SetName(strName);
		MSceneComponent* pSceneComponent = m_pScene->AddComponent<MSceneComponent>(pCameraEntity);
		MCameraComponent* pCameraComponent = m_pScene->AddComponent<MCameraComponent>(pCameraEntity);

		pCameraComponent->SetCameraType(MECameraType::EPerspective);
		pCameraComponent->SetZNear(pCamera->mClipPlaneNear);
		pCameraComponent->SetZFar(pCamera->mClipPlaneFar);
		pCameraComponent->SetFov(pCamera->mHorizontalFOV);

		pSceneComponent->SetPosition(GetVector3(pCamera->mPosition));
		pSceneComponent->LookAt(GetVector3(pCamera->mLookAt), GetVector3(pCamera->mUp));
		
		pEntitySystem->AddChild(GetEntityFromNode(pScene, pScene->mRootNode), pCameraEntity);
	}
}

void MModelConverter::ProcessAnimation(const aiScene* pScene)
{
	if (!m_pSkeletonResource)
	{
		return;
	}

	MResourceSystem* pResourceSystem = GetEngine()->FindSystem<MResourceSystem>();

	MSkeleton* pSkeleton = m_pSkeletonResource->GetSkeleton();
	if (!pSkeleton)
	{
		MORTY_ASSERT(pSkeleton);
		return;
	}

	for (uint32_t i = 0; i < pScene->mNumAnimations; ++i)
	{
		aiAnimation* pAnimation = pScene->mAnimations[i];

		MSkeletalAnimation animationData;

		//animationData.SetSkeletonTemplate(m_pSkeleton);
//		animationData.m_vSkeletalAnimNodes.resize(pAnimation->mNumChannels);// init by nullptr
		animationData.m_vSkeletalAnimNodes.resize(pSkeleton->GetAllBones().size());

		animationData.m_unIndex = i;
		animationData.m_strName = pAnimation->mName.C_Str();
		animationData.m_fTicksDuration = pAnimation->mDuration;
		if (pAnimation->mTicksPerSecond > 0.0f)
			animationData.m_fTicksPerSecond = pAnimation->mTicksPerSecond;

		for (uint32_t chanIndex = 0; chanIndex < pAnimation->mNumChannels; ++chanIndex)
		{
			aiNodeAnim* pNodeAnim = pAnimation->mChannels[chanIndex];

			if (MBone* pBone = pSkeleton->FindBoneByName(pNodeAnim->mNodeName.C_Str()))
			{
				MSkeletalAnimNode& mAnimNode = animationData.m_vSkeletalAnimNodes[pBone->unIndex];

				if (pNodeAnim->mNumPositionKeys > 0)
				{
					for (unsigned keyIndex = 0; keyIndex < pNodeAnim->mNumPositionKeys; ++keyIndex)
					{
						const aiVectorKey& skey = pNodeAnim->mPositionKeys[keyIndex];
						mAnimNode.m_vPositionTrack.push_back({ static_cast<float>(skey.mTime), mfbs::Vector3(skey.mValue.x, skey.mValue.y, skey.mValue.z) });
					}
				}
				if (pNodeAnim->mNumRotationKeys > 0)
				{
					for (unsigned keyIndex = 0; keyIndex < pNodeAnim->mNumRotationKeys; ++keyIndex)
					{
						const aiQuatKey& skey = pNodeAnim->mRotationKeys[keyIndex];
						mAnimNode.m_vRotationTrack.push_back({ static_cast<float>(skey.mTime), mfbs::Quaternion(skey.mValue.w, skey.mValue.x, skey.mValue.y,skey.mValue.z) });
					}
				}
				if (pNodeAnim->mNumScalingKeys > 0)
				{
					for (unsigned keyIndex = 0; keyIndex < pNodeAnim->mNumScalingKeys; ++keyIndex)
					{
						const aiVectorKey& skey = pNodeAnim->mScalingKeys[keyIndex];
						mAnimNode.m_vScaleTrack.push_back({ static_cast<float>(skey.mTime), mfbs::Vector3(skey.mValue.x, skey.mValue.y, skey.mValue.z) });
					}
				}
			}
		}


		std::shared_ptr<MSkeletalAnimationResource> pAnimationResource = pResourceSystem->CreateResource<MSkeletalAnimationResource>();

		std::unique_ptr<MResourceData> resourceData = std::make_unique<MSkeletalAnimationResourceData>();
		if (auto pAnimationResourceData = static_cast<MSkeletalAnimationResourceData*>(resourceData.get()))
		{
			pAnimationResourceData->skeletonAnimation = animationData;
		}
		pAnimationResource->Load(std::move(resourceData));
		pAnimationResource->SetSkeletonResource(m_pSkeletonResource);

		m_vSkeletalAnimation.push_back(pAnimationResource);

	}
}

void MModelConverter::ProcessMaterial(const aiScene* pScene, const uint32_t& nMaterialIdx)
{
	MResourceSystem* pResourceSystem = GetEngine()->FindSystem<MResourceSystem>();


	std::shared_ptr<MMaterialResource> pMaterial = nullptr;
	if (eMaterialType == MModelConvertMaterialType::E_PBR_Deferred)
	{
		std::shared_ptr<MResource> pMeshVSResource = pResourceSystem->LoadResource("Shader/Model/universal_model.mvs");
		std::shared_ptr<MResource> pMeshPSResource = pResourceSystem->LoadResource("Shader/Deferred/deferred_gbuffer.mps");

		if (m_pSkeletonResource)
		{
			const auto pTemplate = pResourceSystem->LoadResource(MMaterialName::DEFERRED_GBUFFER_SKELETON);
			pMaterial = MMaterialResource::CreateMaterial(pTemplate);
		}
		else
		{
			const auto pTemplate = pResourceSystem->LoadResource(MMaterialName::DEFERRED_GBUFFER);
			pMaterial = MMaterialResource::CreateMaterial(pTemplate);
		}

		pMaterial->GetMaterialPropertyBlock()->SetValue(MShaderPropertyName::MATERIAL_METALLIC, 1.0f);
		pMaterial->GetMaterialPropertyBlock()->SetValue(MShaderPropertyName::MATERIAL_ROUGHNESS, 1.0f);
		pMaterial->GetMaterialPropertyBlock()->SetValue(MShaderPropertyName::MATERIAL_ALBEDO, Vector4(1.0f, 1.0f, 1.0f, 1.0f));
		pMaterial->SetTexture(MShaderPropertyName::MATERIAL_TEXTURE_ALBEDO, pResourceSystem->LoadResource(MRenderModule::DefaultWhite));
		pMaterial->SetTexture(MShaderPropertyName::MATERIAL_TEXTURE_NORMAL, pResourceSystem->LoadResource(MRenderModule::DefaultNormal));
		pMaterial->SetTexture(MShaderPropertyName::MATERIAL_TEXTURE_METALLIC, pResourceSystem->LoadResource(MRenderModule::Default_R8_One));
		pMaterial->SetTexture(MShaderPropertyName::MATERIAL_TEXTURE_ROUGHNESS, pResourceSystem->LoadResource(MRenderModule::Default_R8_One));
		pMaterial->SetTexture(MShaderPropertyName::MATERIAL_TEXTURE_AMBIENTOCC, pResourceSystem->LoadResource(MRenderModule::Default_R8_One));
		pMaterial->SetTexture(MShaderPropertyName::MATERIAL_TEXTURE_HEIGHT, pResourceSystem->LoadResource(MRenderModule::Default_R8_Zero));
	}
	else
	{
		std::shared_ptr<MResource> pMeshVSResource = pResourceSystem->LoadResource("Shader/Model/universal_model.mvs");
		std::shared_ptr<MResource> pMeshPSResource = pResourceSystem->LoadResource("Shader/Forward/basic_lighting.mps");

		if (m_pSkeletonResource)
		{
			const auto pTemplate = pResourceSystem->LoadResource(MMaterialName::BASIC_LIGHTING_SKELETON);
			pMaterial = MMaterialResource::CreateMaterial(pTemplate);
		}
		else
		{
			const auto pTemplate = pResourceSystem->LoadResource(MMaterialName::BASIC_LIGHTING);
			pMaterial = MMaterialResource::CreateMaterial(pTemplate);
		}

		std::shared_ptr<MResource> pDefaultTexture = pResourceSystem->LoadResource(MRenderModule::DefaultWhite);
		for (size_t i = 0; i < pMaterial->GetMaterialPropertyBlock()->m_vTextures.size(); ++i)
		{
			pMaterial->SetTexture(pMaterial->GetMaterialPropertyBlock()->m_vTextures[i]->strName, pDefaultTexture);
		}

	}

	if (nMaterialIdx >= pScene->mNumMaterials)
		return;

	aiMaterial* pAiMaterial = pScene->mMaterials[nMaterialIdx];

	if (eMaterialType == MModelConvertMaterialType::E_Default_Forward)
	{
		Vector3 v3Ambient(1.0f, 1.0f, 1.0f), v3Diffuse(1.0f, 1.0f, 1.0f), v3Specular(1.0f, 1.0f, 1.0f);
		float fShininess = 32.0f;

		static const aiString strAmbient("$clr.ambient"), strDiffuse("$clr.diffuse"), strSpecular("$clr.specular"), strShininess("$mat.shininess");
		for (size_t n = 0; n < pAiMaterial->mNumProperties; ++n)
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

		pMaterial->GetMaterialPropertyBlock()->SetValue(MShaderPropertyName::MATERIAL_AMBIENT, v3Ambient);
		pMaterial->GetMaterialPropertyBlock()->SetValue(MShaderPropertyName::MATERIAL_DIFFUSE, v3Diffuse);
		pMaterial->GetMaterialPropertyBlock()->SetValue(MShaderPropertyName::MATERIAL_SPECULAR, v3Specular);
		pMaterial->GetMaterialPropertyBlock()->SetValue(MShaderPropertyName::MATERIAL_SHININESS, fShininess);
		pMaterial->GetMaterialPropertyBlock()->SetValue(MShaderPropertyName::MATERIAL_NORMAL_TEXTURE_ENABLE, 0);
		pMaterial->GetMaterialPropertyBlock()->SetValue(MShaderPropertyName::MATERIAL_ALPHA_FACTOR, 1.0f);

	}

	const std::map<aiTextureType, const MStringId&>* pTextureMapping = nullptr;


	if (eMaterialType == MModelConvertMaterialType::E_Default_Forward)
	{
		static const std::map<aiTextureType, const MStringId&> ForwardTextureMapping = {
			//forward
			{aiTextureType_DIFFUSE, MShaderPropertyName::MATERIAL_TEXTURE_DIFFUSE},
			{aiTextureType_NORMALS, MShaderPropertyName::MATERIAL_TEXTURE_NORMAL},
			{aiTextureType_SPECULAR, MShaderPropertyName::MATERIAL_TEXTURE_SPECULAR}
		};

		pTextureMapping = &ForwardTextureMapping;
	}
	else if (eMaterialType == MModelConvertMaterialType::E_PBR_Deferred)
	{
		static const std::map<aiTextureType, const MStringId&> PbrTextureMapping = {
			{aiTextureType_DIFFUSE, MShaderPropertyName::MATERIAL_TEXTURE_ALBEDO},
			{aiTextureType_NORMALS, MShaderPropertyName::MATERIAL_TEXTURE_NORMAL},

			{aiTextureType_BASE_COLOR, MShaderPropertyName::MATERIAL_TEXTURE_ALBEDO},
			{aiTextureType_NORMAL_CAMERA, MShaderPropertyName::MATERIAL_TEXTURE_NORMAL},
			{aiTextureType_METALNESS, MShaderPropertyName::MATERIAL_TEXTURE_METALLIC},
			{aiTextureType_DIFFUSE_ROUGHNESS, MShaderPropertyName::MATERIAL_TEXTURE_ROUGHNESS},
			{aiTextureType_AMBIENT_OCCLUSION, MShaderPropertyName::MATERIAL_TEXTURE_AMBIENTOCC},
			{aiTextureType_EMISSION_COLOR, MShaderPropertyName::MATERIAL_TEXTURE_EMISSION},
		};

		pTextureMapping = &PbrTextureMapping;
	}
	else
	{
		MORTY_ASSERT(false);
	}

	for (auto pr : *pTextureMapping)
	{
		aiString aiTextureFileName;
		pAiMaterial->GetTexture(pr.first, 0, &aiTextureFileName);

		MString strTextureFileName = aiTextureFileName.C_Str();
		if (strTextureFileName.empty())
		{
			continue;
		}

		auto findResult = m_tRawTextures.find(aiTextureFileName.C_Str());
		if (findResult != m_tRawTextures.end())
		{
			std::shared_ptr<MTextureResource>& pTexture = findResult->second;

			pMaterial->SetTexture(pr.second, pTexture);
		}
		else
		{
			MString strFullPath = MFileHelper::GetFileFolder(m_strResourcePath) + "/" + strTextureFileName;
			std::shared_ptr<MResource> pTexture = nullptr;
			if (m_pTextureDelegate)
			{
				pTexture = m_pTextureDelegate->GetTexture(strFullPath, TextureUsageMapping.at(pr.first));
			}
			else
			{
				auto pTextureData = pResourceSystem->LoadResourceData(strFullPath);
				pTexture = pResourceSystem->CreateResource<MReadableTextureResource>(strFullPath);
				pTexture->Load(std::move(pTextureData));
			}

			pMaterial->SetTexture(pr.second, pTexture);
			m_tFileTextures.insert(pTexture);
		}
	}

	m_vMaterials[nMaterialIdx] = pMaterial;
}

void MModelConverter::ProcessTexture(const aiScene* pScene)
{
	MResourceSystem* pResourceSystem = GetEngine()->FindSystem<MResourceSystem>();

	for (size_t nTextureIdx = 0; nTextureIdx < pScene->mNumTextures; ++nTextureIdx)
	{
		if (aiTexture* aiTexture = pScene->mTextures[nTextureIdx])
		{
			auto pTextureResource = pResourceSystem->CreateResource<MTextureResource>();

			//embedded texture
			if (aiTexture->mHeight == 0)
			{
				pTextureResource->Load(MTextureResourceUtil::ImportTextureFromMemory(MSpan<MByte>{ reinterpret_cast<MByte*>(aiTexture->pcData), aiTexture->mWidth }, MTextureImportInfo(MTexturePixelType::Byte8)));
			}
			else
			{
				const size_t nWidth = aiTexture->mWidth;
				const size_t nHeight = aiTexture->mHeight;
				const size_t nSize = nWidth * nHeight * 4;
				std::vector<MByte> buffer(nSize);

				memcpy(buffer.data(), aiTexture->pcData, nSize);

				MByte temp = 0;
				for (size_t i = 0; i < nSize; i += 4)
				{
					temp = buffer[i];
					buffer[i] = buffer[i + 1];
					buffer[i + 1] = buffer[i + 2];
					buffer[i + 2] = buffer[i + 3];
					buffer[i + 3] = temp;
				}

				pTextureResource->Load(MTextureResourceUtil::LoadFromMemory("RawTexture", buffer, static_cast<uint32_t>(nWidth), static_cast<uint32_t>(nHeight), 4, MTexturePixelType::Byte8));
			}

			m_tRawTextures[aiTexture->mFilename.C_Str()] = pTextureResource;
		}
	}
}

MEntity* MModelConverter::GetEntityFromNode(const aiScene* pScene, aiNode* pNode)
{
	MEntitySystem* pEntitySystem = GetEngine()->FindSystem<MEntitySystem>();

	if (m_tNodeMaps.find(pNode) != m_tNodeMaps.end())
		return m_tNodeMaps[pNode];

	Matrix4 matTransform;
	CopyMatrix4(&matTransform, &pNode->mTransformation);

	MEntity* pEntity = m_pScene->CreateEntity();
	MSceneComponent* pSceneComponent = m_pScene->AddComponent<MSceneComponent>(pEntity);

	pEntity->SetName(pNode->mName.C_Str());
	pSceneComponent->SetTransform(MTransform(matTransform));

	if (pNode->mParent)
	{
		if (MEntity* pParentEntity = GetEntityFromNode(pScene, pNode->mParent))
		{
			pEntitySystem->AddChild(pParentEntity, pEntity);
		}
	}

	m_tNodeMaps[pNode] = pEntity;

	return pEntity;
}

std::shared_ptr<MMaterialResource> MModelConverter::GetMaterial(const aiScene* pScene, const uint32_t& nMaterialIdx)
{
	if (m_vMaterials.size() <= nMaterialIdx)
		m_vMaterials.resize(static_cast<size_t>(nMaterialIdx) + 1);

	if (!m_vMaterials[nMaterialIdx])
		ProcessMaterial(pScene, nMaterialIdx);

	return m_vMaterials[nMaterialIdx];
}
