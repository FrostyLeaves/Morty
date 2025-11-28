/**
 * @File         MModelImporter
 *
 * @Created      2025-01-14
 *
 * @Author       DoubleYe
**/

#include "Tools/MModelImporter.h"
#include "Tools/MMeshImporter.h"

#include "Engine/MEngine.h"
#include "Scene/MScene.h"
#include "Utility/MFileHelper.h"
#include "Utility/MLogger.h"
#include "Utility/MMaterialName.h"
#include "Utility/MTimer.h"

#include "Resource/MEntityResource.h"
#include "Resource/MMaterialResource.h"
#include "Resource/MMeshResource.h"
#include "Resource/MReadableTextureResource.h"
#include "Resource/MSkeletalAnimationResource.h"
#include "Resource/MSkeletonResource.h"
#include "Resource/MTextureResource.h"
#include "Resource/MTextureResourceUtil.h"

#include "Model/MSkeletalAnimation.h"
#include "Model/MSkeleton.h"
#include "Utility/MBounds.h"

#include "Component/MCameraComponent.h"
#include "Component/MComponent.h"
#include "Component/MDirectionalLightComponent.h"
#include "Component/MModelComponent.h"
#include "Component/MPointLightComponent.h"
#include "Component/MRenderMeshComponent.h"
#include "Component/MSceneComponent.h"
#include "Component/MSpotLightComponent.h"

#include "System/MEntitySystem.h"
#include "System/MObjectSystem.h"
#include "System/MResourceSystem.h"

#include "assimp/Importer.hpp"
#include "assimp/material.h"
#include "assimp/postprocess.h"
#include "assimp/scene.h"

#include "MRenderModule.h"

#include <fstream>

using namespace morty;

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

static void CopyMatrix4(Matrix4* matdest, aiMatrix4x4* matsour)
{
    for (uint32_t r = 0; r < 4; ++r)
    {
        for (uint32_t c = 0; c < 4; ++c) { matdest->m[r][c] = (*matsour)[r][c]; }
    }
}

static void CopyMatrix4Transposed(Matrix4* matdest, aiMatrix4x4* matsour)
{
    for (uint32_t r = 0; r < 4; ++r)
    {
        for (uint32_t c = 0; c < 4; ++c) { matdest->m[r][c] = (*matsour)[c][r]; }
    }
}

static MColor  GetColor(const aiColor3D& color) { return MColor(color.r, color.g, color.b); }

static Vector3 GetVector3(const aiVector3D& val) { return Vector3(val.x, val.y, val.z); }

MModelImporter::MModelImporter(MEngine* pEngine)
    : m_engine(pEngine)
    , m_scene(nullptr)
    , m_meshImporter(nullptr)
    , m_meshes()
    , m_skeletonResource(nullptr)
    , m_modelEntity(nullptr)
    , m_skeletalAnimation()
{
    if (m_engine) { m_meshImporter = std::make_unique<MMeshImporter>(m_engine); }
}

MModelImporter::~MModelImporter()
{
    if (m_skeletonResource) { m_skeletonResource = nullptr; }

    m_meshes.clear();
    m_skeletalAnimation.clear();
    m_meshImporter = nullptr;
}

bool MModelImporter::Import(const MModelConvertInfo& convertInfo)
{
    m_convertInfo = convertInfo;

    auto pObjectSystem  = GetEngine()->FindSystem<MObjectSystem>();
    auto resourceSystem = GetEngine()->FindSystem<MResourceSystem>();
    m_scene             = pObjectSystem->CreateObject<MScene>();
    m_defaultMaterial   = resourceSystem->CreateResource<MMaterialTemplateResource>();
    m_defaultMaterial->LoadShader("ShaderSlang/Main/DeferredGBuffer.slang");
    m_defaultMaterial->SetPass(
            MRenderGlobal::DEFAULT_PASS_NAME,
            MRenderGlobal::DEFAULT_VERTEX_ENTRY,
            MRenderGlobal::DEFAULT_PIXEL_ENTRY
    );

    auto time = MTimer::GetCurTime();
    if (!Load(convertInfo.strResourcePath)) { return false; }

    time = MTimer::GetCurTime() - time;
    GetEngine()->GetLogger()->Log("Load Model Time: {}ms", time);

    return SaveResources(convertInfo.strOutputDir, MFileHelper::GetFileName(convertInfo.strResourcePath));
}

bool MModelImporter::Load(const MString& strResourcePath)
{
    MResourceSystem*     pResourceSystem = GetEngine()->FindSystem<MResourceSystem>();

    std::vector<MString> vSearchPath = pResourceSystem->GetSearchPath();

    Assimp::Importer     importer;
    const aiScene*       scene = nullptr;

    for (const MString& strSearchPath: vSearchPath)
    {
        std::string   strFullpath = strSearchPath + strResourcePath;
        std::ifstream ifs(strFullpath.c_str(), std::ios::binary);
        if (!ifs.good()) { continue; }

        scene = importer.ReadFile(
                strFullpath,
                aiProcess_JoinIdenticalVertices | aiProcess_Triangulate | aiProcess_FlipUVs |
                        aiProcess_CalcTangentSpace | aiProcess_FixInfacingNormals | aiProcess_ConvertToLeftHanded
        );

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        {
            GetEngine()->GetLogger()->Error("ERROR::ASSIMP:: {}", importer.GetErrorString());
            return false;
        }
        break;
    }

    if (!scene) { return false; }

    m_modelEntity                    = GetEntityFromNode(scene, scene->mRootNode);
    MModelComponent* pModelComponent = m_scene->AddComponent<MModelComponent>(m_modelEntity);

    // Textures
    ProcessTexture(scene);

    // Bones
    ProcessBones(scene);
    pModelComponent->SetSkeletonResource(m_skeletonResource);

    // Meshes
    ProcessNode(scene->mRootNode, scene);

    // Animation
    ProcessAnimation(scene);

    if (m_convertInfo.bImportLights)
    {
        // Lights
        ProcessLights(scene);
    }

    if (m_convertInfo.bImportCamera)
    {
        // Cameras
        ProcessCameras(scene);
    }

    return true;
}

void MModelImporter::ProcessNode(aiNode* pNode, const aiScene* pScene)
{
    MEntitySystem* pEntitySystem = GetEngine()->FindSystem<MEntitySystem>();

    MSkeleton*     pSkeleton = m_skeletonResource ? m_skeletonResource->GetSkeleton() : nullptr;

    for (uint32_t i = 0; i < pNode->mNumMeshes; ++i)
    {
        aiMesh*                        pChildMesh = pScene->mMeshes[pNode->mMeshes[i]];

        // Use MMeshImporter to import mesh
        MString                        strMeshName;
        std::shared_ptr<MMeshResource> pChildMeshResource =
                m_meshImporter->ImportMesh(pChildMesh, pSkeleton, strMeshName);

        if (!pChildMeshResource) { continue; }

        m_meshes.push_back({strMeshName, pChildMeshResource});

        // Create entity and components
        MEntity* pChildEntity = m_scene->CreateEntity();
        pChildEntity->SetName(pChildMesh->mName.C_Str());
        pChildEntity->RegisterComponent<MSceneComponent>();

        MRenderMeshComponent* meshComponent = pChildEntity->RegisterComponent<MRenderMeshComponent>();
        meshComponent->Load(pChildMeshResource);
        meshComponent->SetMaterial(GetMaterial(pScene, pChildMesh->mMaterialIndex));

        pEntitySystem->AddChild(GetEntityFromNode(pScene, pNode), pChildEntity);
    }

    for (uint32_t i = 0; i < pNode->mNumChildren; ++i)
    {
        aiNode* pChild = pNode->mChildren[i];
        ProcessNode(pChild, pScene);
    }
}

void MModelImporter::ProcessBones(const aiScene* pScene)
{
    std::unique_ptr<MResourceData> pResourceData = std::make_unique<MSkeletonResourceData>();
    MSkeletonResourceData*         pSkeletonData = static_cast<MSkeletonResourceData*>(pResourceData.get());

    RecordBones(&pSkeletonData->skeleton, pScene->mRootNode, pScene);
    BindBones(&pSkeletonData->skeleton, pScene->mRootNode, pScene);

    if (pSkeletonData->skeleton.GetAllBones().empty()) { return; }

    MResourceSystem* pResourceSystem = GetEngine()->FindSystem<MResourceSystem>();
    m_skeletonResource               = pResourceSystem->CreateResource<MSkeletonResource>();

    pSkeletonData->skeleton.SortByDeep();
    m_skeletonResource->Load(std::move(pResourceData));
}

void MModelImporter::RecordBones(MSkeleton* pSkeleton, aiNode* pNode, const aiScene* pScene)
{
    for (uint32_t i = 0; i < pNode->mNumMeshes; ++i)
    {
        aiMesh* pMesh = pScene->mMeshes[pNode->mMeshes[i]];
        if (pMesh->HasBones())
        {
            for (uint32_t j = 0; j < pMesh->mNumBones; ++j)
            {
                if (aiBone* pBone = pMesh->mBones[j])
                {
                    MString strBoneName(pBone->mName.data);
                    MBone*  pMBone = pSkeleton->FindBoneByName(strBoneName);
                    if (nullptr == pMBone)
                    {
                        pMBone = pSkeleton->AppendBone(strBoneName);
                        CopyMatrix4Transposed(&pMBone->m_matOffsetMatrix, &pBone->mOffsetMatrix);
                    }
                }
            }
        }
    }

    for (uint32_t i = 0; i < pNode->mNumChildren; ++i) { RecordBones(pSkeleton, pNode->mChildren[i], pScene); }
}

void MModelImporter::BindBones(MSkeleton* pSkeleton, aiNode* pNode, const aiScene* pScene, MBone* pParent)
{
    MBone* pMBone = pSkeleton->FindBoneByName(pNode->mName.data);
    if (pMBone)
    {
        if (pParent)
        {
            pMBone->unParentIndex = pParent->unIndex;
            pParent->vChildrenIndices.push_back(pMBone->unIndex);
        }
        else { pMBone->unParentIndex = MGlobal::M_INVALID_UINDEX; }

        CopyMatrix4(&pMBone->m_matTransform, &pNode->mTransformation);
    }

    for (uint32_t i = 0; i < pNode->mNumChildren; ++i) { BindBones(pSkeleton, pNode->mChildren[i], pScene, pMBone); }
}

void MModelImporter::ProcessLights(const aiScene* pScene)
{
    for (uint32_t i = 0; i < pScene->mNumLights; ++i)
    {
        aiLight* pLight = pScene->mLights[i];

        MString  strName = pLight->mName.C_Str();

        MEntity* pLightEntity = m_scene->CreateEntity();
        pLightEntity->SetName(strName);

        switch (pLight->mType)
        {
            case aiLightSourceType::aiLightSource_POINT: {
                MSceneComponent*      pSceneComponent = m_scene->AddComponent<MSceneComponent>(pLightEntity);
                MPointLightComponent* pLightComponent = m_scene->AddComponent<MPointLightComponent>(pLightEntity);

                pSceneComponent->SetPosition(GetVector3(pLight->mPosition));
                pLightComponent->SetColor(GetColor(pLight->mColorDiffuse));
                break;
            }

            case aiLightSourceType::aiLightSource_DIRECTIONAL: {
                MSceneComponent*            pSceneComponent = m_scene->AddComponent<MSceneComponent>(pLightEntity);
                MDirectionalLightComponent* pLightComponent =
                        m_scene->AddComponent<MDirectionalLightComponent>(pLightEntity);

                pSceneComponent->SetPosition(GetVector3(pLight->mPosition));
                pSceneComponent->LookAt(GetVector3(pLight->mDirection), GetVector3((pLight->mUp)));
                pLightComponent->SetColor(GetColor(pLight->mColorDiffuse));
                break;
            }

            case aiLightSourceType::aiLightSource_SPOT: {
                MSceneComponent*     pSceneComponent = m_scene->AddComponent<MSceneComponent>(pLightEntity);
                MSpotLightComponent* pLightComponent = m_scene->AddComponent<MSpotLightComponent>(pLightEntity);

                pSceneComponent->SetPosition(GetVector3(pLight->mPosition));
                pSceneComponent->LookAt(GetVector3(pLight->mDirection), GetVector3((pLight->mUp)));
                pLightComponent->SetColor(GetColor(pLight->mColorDiffuse));

                pLightComponent->SetInnerCutOff(pLight->mAngleInnerCone);
                pLightComponent->SetOuterCutOff(pLight->mAngleOuterCone);
                break;
            }

            default: break;
        }
    }
}

void MModelImporter::ProcessCameras(const aiScene* pScene)
{
    MEntitySystem* pEntitySystem = GetEngine()->FindSystem<MEntitySystem>();

    for (uint32_t i = 0; i < pScene->mNumCameras; ++i)
    {
        aiCamera* pCamera = pScene->mCameras[i];
        MString   strName = pCamera->mName.C_Str();

        MEntity*  pCameraEntity = m_scene->CreateEntity();
        pCameraEntity->SetName(strName);
        MSceneComponent*  pSceneComponent  = m_scene->AddComponent<MSceneComponent>(pCameraEntity);
        MCameraComponent* pCameraComponent = m_scene->AddComponent<MCameraComponent>(pCameraEntity);

        pCameraComponent->SetCameraType(MECameraType::EPerspective);
        pCameraComponent->SetZNear(pCamera->mClipPlaneNear);
        pCameraComponent->SetZFar(pCamera->mClipPlaneFar);
        pCameraComponent->SetFov(pCamera->mHorizontalFOV);

        pSceneComponent->SetPosition(GetVector3(pCamera->mPosition));
        pSceneComponent->LookAt(GetVector3(pCamera->mLookAt), GetVector3(pCamera->mUp));

        pEntitySystem->AddChild(GetEntityFromNode(pScene, pScene->mRootNode), pCameraEntity);
    }
}

void MModelImporter::ProcessAnimation(const aiScene* pScene)
{
    if (!m_skeletonResource) { return; }

    MResourceSystem* pResourceSystem = GetEngine()->FindSystem<MResourceSystem>();

    MSkeleton*       pSkeleton = m_skeletonResource->GetSkeleton();
    if (!pSkeleton)
    {
        MORTY_ASSERT(pSkeleton);
        return;
    }

    for (uint32_t i = 0; i < pScene->mNumAnimations; ++i)
    {
        aiAnimation*       pAnimation = pScene->mAnimations[i];

        MSkeletalAnimation animationData;

        animationData.m_skeletalAnimNodes.resize(pSkeleton->GetAllBones().size());

        animationData.m_unIndex       = i;
        animationData.m_strName       = pAnimation->mName.C_Str();
        animationData.m_ticksDuration = pAnimation->mDuration;
        if (pAnimation->mTicksPerSecond > 0.0f) { animationData.m_ticksPerSecond = pAnimation->mTicksPerSecond; }

        for (uint32_t chanIndex = 0; chanIndex < pAnimation->mNumChannels; ++chanIndex)
        {
            aiNodeAnim* pNodeAnim = pAnimation->mChannels[chanIndex];

            if (MBone* pBone = pSkeleton->FindBoneByName(pNodeAnim->mNodeName.C_Str()))
            {
                MSkeletalAnimNode& mAnimNode = animationData.m_skeletalAnimNodes[pBone->unIndex];

                if (pNodeAnim->mNumPositionKeys > 0)
                {
                    for (unsigned keyIndex = 0; keyIndex < pNodeAnim->mNumPositionKeys; ++keyIndex)
                    {
                        const aiVectorKey& skey = pNodeAnim->mPositionKeys[keyIndex];
                        mAnimNode.m_positionTrack.push_back(
                                {static_cast<float>(skey.mTime),
                                 fbs::Vector3(skey.mValue.x, skey.mValue.y, skey.mValue.z)}
                        );
                    }
                }

                if (pNodeAnim->mNumRotationKeys > 0)
                {
                    for (unsigned keyIndex = 0; keyIndex < pNodeAnim->mNumRotationKeys; ++keyIndex)
                    {
                        const aiQuatKey& skey = pNodeAnim->mRotationKeys[keyIndex];
                        mAnimNode.m_rotationTrack.push_back(
                                {static_cast<float>(skey.mTime),
                                 fbs::Quaternion(skey.mValue.w, skey.mValue.x, skey.mValue.y, skey.mValue.z)}
                        );
                    }
                }

                if (pNodeAnim->mNumScalingKeys > 0)
                {
                    for (unsigned keyIndex = 0; keyIndex < pNodeAnim->mNumScalingKeys; ++keyIndex)
                    {
                        const aiVectorKey& skey = pNodeAnim->mScalingKeys[keyIndex];
                        mAnimNode.m_scaleTrack.push_back(
                                {static_cast<float>(skey.mTime),
                                 fbs::Vector3(skey.mValue.x, skey.mValue.y, skey.mValue.z)}
                        );
                    }
                }
            }
        }

        std::shared_ptr<MSkeletalAnimationResource> pAnimationResource =
                pResourceSystem->CreateResource<MSkeletalAnimationResource>();

        std::unique_ptr<MResourceData> resourceData = std::make_unique<MSkeletalAnimationResourceData>();
        if (auto* pAnimationResourceData = static_cast<MSkeletalAnimationResourceData*>(resourceData.get()))
        {
            pAnimationResourceData->skeletonAnimation = animationData;
        }
        pAnimationResource->Load(std::move(resourceData));
        pAnimationResource->SetSkeletonResource(m_skeletonResource);

        m_skeletalAnimation.push_back(pAnimationResource);
    }
}

void MModelImporter::ProcessMaterial(const aiScene* pScene, const uint32_t& nMaterialIdx)
{
    MResourceSystem*                   pResourceSystem = GetEngine()->FindSystem<MResourceSystem>();

    std::shared_ptr<MMaterialResource> material = nullptr;

    material = MMaterialResource::CreateMaterial(m_defaultMaterial);

    material->SetValue(MShaderPropertyName::MATERIAL_METALLIC, 1.0f);
    material->SetValue(MShaderPropertyName::MATERIAL_ROUGHNESS, 1.0f);
    material->SetValue(MShaderPropertyName::MATERIAL_ALBEDO, Vector3(1.0f, 1.0f, 1.0f));
    material->SetTexture(
            MShaderPropertyName::MATERIAL_TEXTURE_ALBEDO,
            pResourceSystem->LoadResource(MRenderModule::DefaultWhite)
    );
    material->SetTexture(
            MShaderPropertyName::MATERIAL_TEXTURE_NORMAL,
            pResourceSystem->LoadResource(MRenderModule::DefaultNormal)
    );
    material->SetTexture(
            MShaderPropertyName::MATERIAL_TEXTURE_METALLIC,
            pResourceSystem->LoadResource(MRenderModule::Default_R8_One)
    );
    material->SetTexture(
            MShaderPropertyName::MATERIAL_TEXTURE_ROUGHNESS,
            pResourceSystem->LoadResource(MRenderModule::Default_R8_One)
    );
    material->SetTexture(
            MShaderPropertyName::MATERIAL_TEXTURE_AMBIENTOCC,
            pResourceSystem->LoadResource(MRenderModule::Default_R8_One)
    );
    material->SetTexture(
            MShaderPropertyName::MATERIAL_TEXTURE_HEIGHT,
            pResourceSystem->LoadResource(MRenderModule::Default_R8_Zero)
    );


    if (nMaterialIdx >= pScene->mNumMaterials) { return; }

    aiMaterial*                                            pAiMaterial     = pScene->mMaterials[nMaterialIdx];
    const std::map<aiTextureType, const MStringId&>*       pTextureMapping = nullptr;

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

    for (auto pr: *pTextureMapping)
    {
        aiString aiTextureFileName;
        pAiMaterial->GetTexture(pr.first, 0, &aiTextureFileName);

        MString strTextureFileName = aiTextureFileName.C_Str();
        if (strTextureFileName.empty()) { continue; }

        auto findResult = m_rawTextures.find(aiTextureFileName.C_Str());
        if (findResult != m_rawTextures.end())
        {
            std::shared_ptr<MTextureResource>& pTexture = findResult->second;
            material->SetTexture(pr.second, pTexture);
        }
        else
        {
            MString strFullPath = MFileHelper::GetFileFolder(m_convertInfo.strResourcePath) + "/" + strTextureFileName;
            std::shared_ptr<MResource> pTexture = nullptr;

            if (m_convertInfo.pTextureDelegate)
            {
                pTexture = m_convertInfo.pTextureDelegate->GetTexture(strFullPath, TextureUsageMapping.at(pr.first));
            }
            else
            {
                auto pTextureData = pResourceSystem->LoadResourceData(strFullPath);
                pTexture          = pResourceSystem->CreateResource<MReadableTextureResource>(strFullPath);
                pTexture->Load(std::move(pTextureData));
            }

            material->SetTexture(pr.second, pTexture);
            m_fileTextures.insert(pTexture);
        }
    }

    if (m_convertInfo.pMaterialDelegate) { m_convertInfo.pMaterialDelegate->PostProcess(material.get()); }

    m_materials[nMaterialIdx] = material;
}

void MModelImporter::ProcessTexture(const aiScene* pScene)
{
    MResourceSystem* pResourceSystem = GetEngine()->FindSystem<MResourceSystem>();

    for (size_t nTextureIdx = 0; nTextureIdx < pScene->mNumTextures; ++nTextureIdx)
    {
        if (aiTexture* aiTexture = pScene->mTextures[nTextureIdx])
        {
            auto pTextureResource = pResourceSystem->CreateResource<MTextureResource>();

            // Embedded texture
            if (aiTexture->mHeight == 0)
            {
                pTextureResource->Load(
                        MTextureResourceUtil::ImportTextureFromMemory(
                                MSpan<MByte>{reinterpret_cast<MByte*>(aiTexture->pcData), aiTexture->mWidth},
                                MTextureImportInfo(MTexturePixelType::Byte8)
                        )
                );
            }
            else
            {
                const size_t       nWidth  = aiTexture->mWidth;
                const size_t       nHeight = aiTexture->mHeight;
                const size_t       nSize   = nWidth * nHeight * 4;
                std::vector<MByte> buffer(nSize);

                memcpy(buffer.data(), aiTexture->pcData, nSize);

                MByte temp = 0;
                for (size_t i = 0; i < nSize; i += 4)
                {
                    temp          = buffer[i];
                    buffer[i]     = buffer[i + 1];
                    buffer[i + 1] = buffer[i + 2];
                    buffer[i + 2] = buffer[i + 3];
                    buffer[i + 3] = temp;
                }

                pTextureResource->Load(
                        MTextureResourceUtil::LoadFromMemory(
                                "RawTexture",
                                buffer,
                                static_cast<uint32_t>(nWidth),
                                static_cast<uint32_t>(nHeight),
                                4,
                                MTexturePixelType::Byte8
                        )
                );
            }

            m_rawTextures[aiTexture->mFilename.C_Str()] = pTextureResource;
        }
    }
}

MEntity* MModelImporter::GetEntityFromNode(const aiScene* pScene, aiNode* pNode)
{
    MEntitySystem* pEntitySystem = GetEngine()->FindSystem<MEntitySystem>();

    if (m_nodeMaps.find(pNode) != m_nodeMaps.end()) { return m_nodeMaps[pNode]; }

    Matrix4 matTransform;
    CopyMatrix4(&matTransform, &pNode->mTransformation);

    MEntity*         pEntity         = m_scene->CreateEntity();
    MSceneComponent* pSceneComponent = m_scene->AddComponent<MSceneComponent>(pEntity);

    pEntity->SetName(pNode->mName.C_Str());
    pSceneComponent->SetTransform(MTransform(matTransform));

    if (pNode->mParent)
    {
        if (MEntity* pParentEntity = GetEntityFromNode(pScene, pNode->mParent))
        {
            pEntitySystem->AddChild(pParentEntity, pEntity);
        }
    }

    m_nodeMaps[pNode] = pEntity;

    return pEntity;
}

std::shared_ptr<MMaterialResource> MModelImporter::GetMaterial(const aiScene* pScene, const uint32_t& nMaterialIdx)
{
    if (m_materials.size() <= nMaterialIdx) { m_materials.resize(static_cast<size_t>(nMaterialIdx) + 1); }

    if (!m_materials[nMaterialIdx]) { ProcessMaterial(pScene, nMaterialIdx); }

    return m_materials[nMaterialIdx];
}

bool MModelImporter::SaveResources(const MString& strOutputDir, const MString& strOutputName)
{
    MResourceSystem* pResourceSystem = GetEngine()->FindSystem<MResourceSystem>();
    MEntitySystem*   pEntitySystem   = GetEngine()->FindSystem<MEntitySystem>();

    MString          strPath = strOutputDir + "/" + strOutputName + "/";

    MFileHelper::MakeDir(strOutputDir);

    if (m_defaultMaterial)
    {
        pResourceSystem->MoveTo(m_defaultMaterial, strPath + strOutputName + ".mat_temp");
        pResourceSystem->SaveResource(m_defaultMaterial);
    }

    // Save skeleton
    if (m_skeletonResource)
    {
        pResourceSystem->MoveTo(m_skeletonResource, strPath + strOutputName + ".ske");
        pResourceSystem->SaveResource(m_skeletonResource);
    }

    // Save raw textures
    for (auto pr: m_rawTextures)
    {
        if (pr.second)
        {
            std::shared_ptr<MTextureResource> pTextureResource = pr.second;
            MString                           strValidFileName = pr.first;
            pResourceSystem->MoveTo(pTextureResource, strPath + strValidFileName + ".mtex");
            pResourceSystem->SaveResource(pTextureResource);
        }
    }

    // Save file textures
    for (auto pTexture: m_fileTextures)
    {
        if (pTexture)
        {
            MString strValidFileName = MFileHelper::GetFileName(pTexture->GetResourcePath());
            pResourceSystem->MoveTo(pTexture, strPath + strValidFileName + ".mtex");
            pResourceSystem->SaveResource(pTexture);
        }
    }

    // Save materials
    for (size_t i = 0; i < m_materials.size(); ++i)
    {
        if (m_materials[i])
        {
            MString strMaterialFileName = strPath + "material_" + MStringUtil::ToString(i);
            pResourceSystem->MoveTo(m_materials[i], strMaterialFileName + ".mat");
            pResourceSystem->SaveResource(m_materials[i]);
        }
    }

    // Save meshes
    for (size_t i = 0; i < m_meshes.size(); ++i)
    {
        std::shared_ptr<MMeshResource> pMeshResource   = m_meshes[i].second;
        MString                        strMeshFileName = strPath + m_meshes[i].first + "_" + MStringUtil::ToString(i);

        pResourceSystem->MoveTo(pMeshResource, strMeshFileName + ".mesh");
        pResourceSystem->SaveResource(pMeshResource);
    }

    // Save animations
    for (std::shared_ptr<MSkeletalAnimationResource> pAnimResource: m_skeletalAnimation)
    {
        MString strValidFileName = pAnimResource->GetAnimationName();
        MFileHelper::GetValidFileName(strValidFileName);
        pResourceSystem->MoveTo(pAnimResource, strPath + strValidFileName + ".anim");
        pResourceSystem->SaveResource(pAnimResource);
    }

    // Save entity hierarchy
    auto                       vAllEntity    = m_scene->GetAllEntity();
    std::shared_ptr<MResource> pNodeResource = pEntitySystem->PackEntity(vAllEntity);

    pResourceSystem->MoveTo(pNodeResource, strPath + strOutputName + ".entity");
    pResourceSystem->SaveResource(pNodeResource);

    pNodeResource = nullptr;

    return true;
}
