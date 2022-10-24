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
#include "Resource/MModelResource.h"
#include "Resource/MEntityResource.h"
#include "Resource/MTextureResource.h"
#include "Resource/MMaterialResource.h"
#include "Resource/MSkeletonResource.h"
#include "Resource/MSkeletalAnimationResource.h"

#include "Utility/MBounds.h"
#include "Model/MSkeleton.h"
#include "Model/MSkeletalAnimation.h"

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#include "Utility/MFileHelper.h"


#include "Component/MComponent.h"
#include "Component/MModelComponent.h"
#include "Component/MSceneComponent.h"
#include "Component/MCameraComponent.h"
#include "Component/MSpotLightComponent.h"
#include "Component/MPointLightComponent.h"
#include "Component/MRenderableMeshComponent.h"
#include "Component/MDirectionalLightComponent.h"

#include "System/MEntitySystem.h"
#include "System/MObjectSystem.h"
#include "System/MResourceSystem.h"

#include <fstream>

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
	, m_pScene(nullptr)
	, m_strResourcePath()
	, m_vMeshes()
	, m_pSkeleton(nullptr)
	, m_pModelEntity(nullptr)
	, m_vSkeletalAnimation()
	, eMaterialType(MModelConvertMaterialType::E_Default_Forward)
{
}

MModelConverter::~MModelConverter()
{
	if (m_pSkeleton)
	{
		m_pSkeleton = nullptr;
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

	auto time = MTimer::GetCurTime();
	eMaterialType = convertInfo.eMaterialType;
	if (!Load(convertInfo.strResourcePath))
		return false;

	time = MTimer::GetCurTime() - time;

	GetEngine()->GetLogger()->Log("Load Model Time: %lld", time);

	MString strPath = convertInfo.strOutputDir + "/" + convertInfo.strOutputName + "/";

	MFileHelper::MakeDir(convertInfo.strOutputDir + "/" + convertInfo.strOutputName);


	if (m_pSkeleton)
	{
		pResourceSystem->MoveTo(m_pSkeleton, strPath + convertInfo.strOutputName + ".ske");
		m_pSkeleton->Save();
	}

	for (int i = 0; i < m_vMaterials.size(); ++i)
	{
		if (m_vMaterials[i])
		{
			MString strMaterialFileName = strPath + "material_" + MStringHelper::ToString(i);
			pResourceSystem->MoveTo(m_vMaterials[i], strMaterialFileName + ".mat");
			m_vMaterials[i]->Save();
		}
	}

	for (int i = 0; i < m_vMeshes.size(); ++i)
	{
		std::shared_ptr<MMeshResource> pMeshResource = m_vMeshes[i];
		MString strMeshFileName = strPath + pMeshResource->GetMeshName() + "_" + MStringHelper::ToString(i);

		
		pResourceSystem->MoveTo(pMeshResource, strMeshFileName + ".mesh");
		pMeshResource->Save();
	}

	for (std::shared_ptr<MSkeletalAnimation> pAnimResource : m_vSkeletalAnimation)
	{
		MString strValidFileName = pAnimResource->GetName();
		MFileHelper::GetValidFileName(strValidFileName);
		pResourceSystem->MoveTo(pAnimResource, strPath + strValidFileName + ".anim");
		pAnimResource->Save();

	}


	auto&& vAllEntity = m_pScene->GetAllEntity();
	std::shared_ptr<MResource> pNodeResource = pEntitySystem->PackEntity(m_pScene, vAllEntity);

	pResourceSystem->MoveTo(pNodeResource, strPath + convertInfo.strOutputName + ".entity");
	pNodeResource->Save();

	pNodeResource = nullptr;

	return true;
}

bool MModelConverter::Load(const MString& strResourcePath)
{
	MObjectSystem* pObjectSystem = GetEngine()->FindSystem<MObjectSystem>();
	MResourceSystem* pResourceSystem = GetEngine()->FindSystem<MResourceSystem>();

	m_pSkeleton = pResourceSystem->CreateResource<MSkeletonResource>();

	m_pModelEntity = m_pScene->CreateEntity();
	
	MSceneComponent* pSceneComponent = m_pScene->AddComponent<MSceneComponent>(m_pModelEntity);
	MModelComponent* pModelComponent = m_pScene->AddComponent<MModelComponent>(m_pModelEntity);

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
			GetEngine()->GetLogger()->Error("ERROR::ASSIMP:: %s", importer.GetErrorString());
			return false;
		}
		break;
	}

	if (!scene)
		return false;
	

	m_pModelEntity->SetName(scene->mRootNode->mName.C_Str());

	//Textures
	ProcessTexture(scene);

	//Bones
	ProcessBones(scene);
	pModelComponent->SetSkeletonResource(m_pSkeleton);

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
	MEntitySystem* pEntitySystem = GetEngine()->FindSystem<MEntitySystem>();
	MObjectSystem* pObjectSystem = GetEngine()->FindSystem<MObjectSystem>();
	MResourceSystem* pResourceSystem = GetEngine()->FindSystem<MResourceSystem>();

	for (uint32_t i = 0; i < pNode->mNumMeshes; ++i)
	{
		aiMesh* pChildMesh = pScene->mMeshes[pNode->mMeshes[i]];

		std::shared_ptr<MMeshResource> pChildMeshResource = pResourceSystem->CreateResource<MMeshResource>();

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

		pChildMeshResource->m_strName = pChildMesh->mName.C_Str();
		pChildMeshResource->m_SkeletonKeeper.SetResource(m_pSkeleton);
		pChildMeshResource->m_MaterialKeeper.SetResource(GetMaterial(pScene, pChildMesh->mMaterialIndex));
		pChildMeshResource->ResetBounds();
		m_vMeshes.push_back(pChildMeshResource);


		MEntity* pChildEntity = m_pScene->CreateEntity();
		pChildEntity->SetName(pChildMesh->mName.C_Str());
		
		MComponent* pSceneComponent = m_pScene->AddComponent<MSceneComponent>(pChildEntity);
		MRenderableMeshComponent* pMeshComponent = m_pScene->AddComponent<MRenderableMeshComponent>(pChildEntity);
		pMeshComponent->Load(pChildMeshResource);

		pEntitySystem->AddChild(GetEntityFromNode(pScene, pNode), pChildEntity);
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
			pMBone->unParentIndex = MGlobal::M_INVALID_INDEX;
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
	MObjectSystem* pObjectSystem = GetEngine()->FindSystem<MObjectSystem>();

	for (uint32_t i = 0; i < pScene->mNumLights; ++i)
	{
		aiLight* pLight = pScene->mLights[i];

		MString strName = pLight->mName.C_Str();

		MEntity* pLightEntity = m_pScene->CreateEntity();
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

		MEntity* pCameraEntity = m_pScene->CreateEntity();
		MSceneComponent* pSceneComponent = m_pScene->AddComponent<MSceneComponent>(pCameraEntity);
		MCameraComponent* pCameraComponent = m_pScene->AddComponent<MCameraComponent>(pCameraEntity);

		pCameraComponent->SetCameraType(MCameraComponent::MECameraType::EPerspective);
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
	if (!m_pSkeleton)
		return;

	MResourceSystem* pResourceSystem = GetEngine()->FindSystem<MResourceSystem>();

	for (uint32_t i = 0; i < pScene->mNumAnimations; ++i)
	{
		aiAnimation* pAnimation = pScene->mAnimations[i];

		std::shared_ptr<MSkeletalAnimationResource> pMAnimation = pResourceSystem->CreateResource<MSkeletalAnimationResource>();

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
	MResourceSystem* pResourceSystem = GetEngine()->FindSystem<MResourceSystem>();


	std::shared_ptr<MMaterial> pMaterial = nullptr;
	if (eMaterialType == MModelConvertMaterialType::E_PBR_Deferred)
	{
		std::shared_ptr<MResource> pMeshVSResource = pResourceSystem->LoadResource("Shader/model_gbuffer.mvs");
		std::shared_ptr<MResource> pMeshPSResource = pResourceSystem->LoadResource("Shader/model_gbuffer.mps");

		if (m_pSkeleton)
		{
			std::shared_ptr<MMaterialResource> pSkinnedMeshMaterialRes = pResourceSystem->CreateResource<MMaterialResource>();
			pSkinnedMeshMaterialRes->GetShaderMacro().SetInnerMacro(MRenderGlobal::SHADER_SKELETON_ENABLE, "1");
			pSkinnedMeshMaterialRes->LoadVertexShader(pMeshVSResource);
			pSkinnedMeshMaterialRes->LoadPixelShader(pMeshPSResource);
			pMaterial = pSkinnedMeshMaterialRes;
		}
		else
		{
			std::shared_ptr<MMaterialResource> pStaticMeshMaterialRes = pResourceSystem->CreateResource<MMaterialResource>();
			pStaticMeshMaterialRes->LoadVertexShader(pMeshVSResource);
			pStaticMeshMaterialRes->LoadPixelShader(pMeshPSResource);
			pMaterial = pStaticMeshMaterialRes;
		}

		pMaterial->SetMaterialType(MEMaterialType::EDeferred);

		pMaterial->SetTexutre(MaterialKey::Albedo, pResourceSystem->LoadResource(MRenderModule::DefaultWhite));
		pMaterial->SetTexutre(MaterialKey::Normal, pResourceSystem->LoadResource(MRenderModule::DefaultNormal));
		pMaterial->SetTexutre(MaterialKey::Metallic, pResourceSystem->LoadResource(MRenderModule::Default_R8_One));
		pMaterial->SetTexutre(MaterialKey::Roughness, pResourceSystem->LoadResource(MRenderModule::Default_R8_One));
		pMaterial->SetTexutre(MaterialKey::AmbientOcc, pResourceSystem->LoadResource(MRenderModule::Default_R8_One));
		pMaterial->SetTexutre(MaterialKey::Height, pResourceSystem->LoadResource(MRenderModule::Default_R8_Zero));
	}
	else
	{
		std::shared_ptr<MResource> pMeshVSResource = pResourceSystem->LoadResource("Shader/model.mvs");
		std::shared_ptr<MResource> pMeshPSResource = pResourceSystem->LoadResource("Shader/model.mps");

		if (m_pSkeleton)
		{
			std::shared_ptr<MMaterialResource> pSkinnedMeshMaterialRes = pResourceSystem->CreateResource<MMaterialResource>();
			pSkinnedMeshMaterialRes->GetShaderMacro().SetInnerMacro(MRenderGlobal::SHADER_SKELETON_ENABLE, "1");
			pSkinnedMeshMaterialRes->LoadVertexShader(pMeshVSResource);
			pSkinnedMeshMaterialRes->LoadPixelShader(pMeshPSResource);
			pMaterial = pSkinnedMeshMaterialRes;
		}
		else
		{
			std::shared_ptr<MMaterialResource> pStaticMeshMaterialRes = pResourceSystem->CreateResource<MMaterialResource>();
			pStaticMeshMaterialRes->LoadVertexShader(pMeshVSResource);
			pStaticMeshMaterialRes->LoadPixelShader(pMeshPSResource);
			pMaterial = pStaticMeshMaterialRes;
		}

		pMaterial->SetMaterialType(MEMaterialType::EDefault);
	}

	if(nMaterialIdx >= pScene->mNumMaterials)
		return;

	std::shared_ptr<MResource> pDefaultTexture = pResourceSystem->LoadResource(MRenderModule::DefaultWhite);
	for (size_t i = 0; i < pMaterial->GetTextureParams()->size(); ++i)
	{
		pMaterial->SetTexutre(pMaterial->GetTextureParams()->at(i)->strName, pDefaultTexture);
	}

	aiMaterial* pAiMaterial = pScene->mMaterials[nMaterialIdx];

	if (eMaterialType == MModelConvertMaterialType::E_Default_Forward)
	{
		Vector3 v3Ambient(1.0f, 1.0f, 1.0f), v3Diffuse(1.0f, 1.0f, 1.0f), v3Specular(1.0f, 1.0f, 1.0f);
		float fShininess = 32.0f;

		static const aiString strAmbient("$clr.ambient"), strDiffuse("$clr.diffuse"), strSpecular("$clr.specular"), strShininess("$mat.shininess");
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

		MStruct* pMaterialStruct = nullptr;
		for (MShaderConstantParam* pParam : *pMaterial->GetShaderParams())
		{
			if (MStruct* pStruct = pParam->var.GetStruct())
			{
				if (MStruct* pMat = pStruct->GetValue("U_mat")->GetStruct())
				{
					pMaterialStruct = pMat;

					pMat->SetValue("f3Ambient", v3Ambient);
					pMat->SetValue("f3Diffuse", v3Diffuse);
					pMat->SetValue("f3Specular", v3Specular);
					pMat->SetValue("fShininess", fShininess);
					pMat->SetValue("bUseNormalTex", false);
					pMat->SetValue("fAlphaFactor", 1.0f);
				}
			}
		}
	}


	static const std::map<aiTextureType, MString> TextureMapping = {

		//forward
		{aiTextureType_DIFFUSE, "U_mat_texDiffuse"},
		{aiTextureType_NORMALS, "U_mat_texNormal"},
		{aiTextureType_SPECULAR, "U_mat_texSpecular"},


		//pbr
		{aiTextureType_BASE_COLOR, "U_mat_texAlbedo"},
		{aiTextureType_NORMAL_CAMERA, "U_mat_texNormal"},
		{aiTextureType_EMISSION_COLOR, "U_mat_texEmission"},
		{aiTextureType_METALNESS, "U_mat_texMetallic"},
		{aiTextureType_DIFFUSE_ROUGHNESS, "U_mat_texRoughness"},
		{aiTextureType_AMBIENT_OCCLUSION, "U_mat_texAmbientOcc"},
	};


	for (auto&& pr : TextureMapping)
	{
		aiString strTextureFileName;
		pAiMaterial->GetTexture(pr.first, 0, &strTextureFileName);

		auto findResult = m_tTextures.find(strTextureFileName.C_Str());
		if (findResult != m_tTextures.end())
		{
			std::shared_ptr<MTextureResource>& pTexture = findResult->second;

			pMaterial->SetTexutre(pr.second, pTexture);
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
			auto&& pTextureResource = pResourceSystem->CreateResource<MTextureResource>();

			//embedded texture
			if (aiTexture->mHeight == 0)
			{
				pTextureResource->ImportTextureFromMemory(reinterpret_cast<char*>(aiTexture->pcData), aiTexture->mWidth, MTextureResource::ImportInfo(MTextureResource::PixelFormat::Byte8));
			}
			else
			{
				const size_t nWidth = aiTexture->mWidth;
				const size_t nHeight = aiTexture->mHeight;
				const size_t nSize = nWidth * nHeight * 4;
				unsigned char* buffer = new unsigned char[nSize];

				memcpy(buffer, aiTexture->pcData, nSize);

				char temp = 0;
				for (size_t i = 0; i < nSize; i += 4)
				{
					temp = buffer[i];
					buffer[i] = buffer[i + 1];
					buffer[i + 1] = buffer[i + 2];
					buffer[i + 2] = buffer[i + 3];
					buffer[i + 3] = temp;
				}

				pTextureResource->LoadFromMemory(buffer, nWidth, nHeight, 4, MTextureResource::PixelFormat::Byte8, false);

			}

			m_tTextures[aiTexture->mFilename.C_Str()] = pTextureResource;
		}
	}
}

MEntity* MModelConverter::GetEntityFromNode(const aiScene* pScene, aiNode* pNode)
{
	MEntitySystem* pEntitySystem = GetEngine()->FindSystem<MEntitySystem>();

	if (pScene->mRootNode == pNode)
		return m_pModelEntity;

	if (m_tNodeMaps.find(pNode) != m_tNodeMaps.end())
		return m_tNodeMaps[pNode];

	Matrix4 matTransform;
	CopyMatrix4(&matTransform, &pNode->mTransformation);

	MEntity* pParentEntity = GetEntityFromNode(pScene, pNode->mParent);

	MEntity* pEntity = m_pScene->CreateEntity();
	MSceneComponent* pSceneComponent = m_pScene->AddComponent<MSceneComponent>(pEntity);

	pEntity->SetName(pNode->mName.C_Str());
	pSceneComponent->SetTransform(MTransform(matTransform));

	pEntitySystem->AddChild(pParentEntity, pEntity);

	m_tNodeMaps[pNode] = pEntity;

	return pEntity;
}

std::shared_ptr<MMaterial> MModelConverter::GetMaterial(const aiScene* pScene, const uint32_t& nMaterialIdx)
{
	if (m_vMaterials.size() <= nMaterialIdx)
		m_vMaterials.resize(static_cast<size_t>(nMaterialIdx) + 1);

	if (!m_vMaterials[nMaterialIdx])
		ProcessMaterial(pScene, nMaterialIdx);

	return m_vMaterials[nMaterialIdx];
}
