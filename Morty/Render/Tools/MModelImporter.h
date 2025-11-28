/**
 * @File         MModelImporter
 *
 * @Created      2025-01-14
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"
#include "Resource/MMaterialResource.h"
#include "Resource/MMaterialTemplateResource.h"
#include "Resource/MResource.h"
#include "Resource/MSkeletonResource.h"
#include "Utility/MString.h"


#include <map>
#include <memory>
#include <set>
#include <vector>

struct aiNode;
struct aiScene;
struct aiMesh;

namespace morty
{
class MSkeletalAnimationResource;
class MBone;
class MScene;
class MEntity;
class MSkeleton;
class MMeshResource;
class MTextureResource;
class MMeshImporter;
class MEngine;

enum class MEModelTextureUsage
{
    Unknow = 0,
    BaseColor,
    Normal,
    Metallic,
    Roughness,
    AmbientOcc,
    Emission,
};

class MORTY_API MITextureDelegate
{
public:
    virtual ~MITextureDelegate() = default;

    virtual std::shared_ptr<MTextureResource> GetTexture(const MString& strFullPath, MEModelTextureUsage eUsage) = 0;
};

class MORTY_API MIMaterialDelegate
{
public:
    virtual ~MIMaterialDelegate() = default;

    virtual void PostProcess(MMaterial* material) = 0;
};

struct MORTY_API MModelConvertInfo {
    MString                             strResourcePath;
    MString                             strOutputDir;

    bool                                bImportCamera = false;
    bool                                bImportLights = true;

    std::shared_ptr<MITextureDelegate>  pTextureDelegate  = nullptr;
    std::shared_ptr<MIMaterialDelegate> pMaterialDelegate = nullptr;
};

/**
 * @brief Model Importer - Handles importing complete models from Assimp
 *
 * This class is responsible for:
 * - Loading Assimp scene
 * - Processing scene hierarchy (nodes)
 * - Processing materials and textures
 * - Processing skeleton and animations
 * - Processing lights and cameras
 * - Organizing entities and components
 * - Saving all resources
 *
 * Uses MMeshImporter for mesh-specific operations
 */
class MORTY_API MModelImporter
{
public:
    explicit MModelImporter(MEngine* engine);
    virtual ~MModelImporter();

    /**
     * @brief Import and convert a model
     * @param convertInfo Import configuration
     * @return true on success
     */
    bool Import(const MModelConvertInfo& convertInfo);

protected:
    /**
     * @brief Load Assimp scene from file
     */
    bool     Load(const MString& strResourcePath);

    /**
     * @brief Process scene node hierarchy
     */
    void     ProcessNode(aiNode* pNode, const aiScene* scene);

    /**
     * @brief Process skeleton from scene
     */
    void     ProcessBones(const aiScene* scene);

    /**
     * @brief Record bones from scene nodes
     */
    void     RecordBones(MSkeleton* pSkeleton, aiNode* pNode, const aiScene* scene);

    /**
     * @brief Bind bones to hierarchy
     */
    void     BindBones(MSkeleton* pSkeleton, aiNode* pNode, const aiScene* scene, MBone* pParent = nullptr);

    /**
     * @brief Process lights from scene
     */
    void     ProcessLights(const aiScene* scene);

    /**
     * @brief Process cameras from scene
     */
    void     ProcessCameras(const aiScene* scene);

    /**
     * @brief Process skeletal animations
     */
    void     ProcessAnimation(const aiScene* scene);

    /**
     * @brief Process material at index
     */
    void     ProcessMaterial(const aiScene* scene, const uint32_t& nMaterialIdx);

    /**
     * @brief Process embedded textures
     */
    void     ProcessTexture(const aiScene* scene);

    /**
     * @brief Get or create entity from Assimp node
     */
    MEntity* GetEntityFromNode(const aiScene* scene, aiNode* pNode);

    /**
     * @brief Get or create material at index
     */
    std::shared_ptr<MMaterialResource> GetMaterial(const aiScene* scene, const uint32_t& nMaterialIdx);

    /**
     * @brief Save all resources to disk
     */
    bool                               SaveResources(const MString& strOutputDir, const MString& strOutputName);

    MEngine*                           GetEngine() { return m_engine; }

private:
    MEngine*                                                        m_engine;
    MScene*                                                         m_scene;

    MModelConvertInfo                                               m_convertInfo;

    std::unique_ptr<MMeshImporter>                                  m_meshImporter;

    std::vector<std::pair<MString, std::shared_ptr<MMeshResource>>> m_meshes;
    std::vector<std::shared_ptr<MMaterialResource>>                 m_materials;
    std::map<MString, std::shared_ptr<MTextureResource>>            m_rawTextures;
    std::set<std::shared_ptr<MResource>>                            m_fileTextures;

    std::map<aiNode*, MEntity*>                                     m_nodeMaps;

    std::shared_ptr<MMaterialTemplateResource>                      m_defaultMaterial;
    std::shared_ptr<MSkeletonResource>                              m_skeletonResource;
    MEntity*                                                        m_modelEntity;

    std::vector<std::shared_ptr<MSkeletalAnimationResource>>        m_skeletalAnimation;
};
}// namespace morty
