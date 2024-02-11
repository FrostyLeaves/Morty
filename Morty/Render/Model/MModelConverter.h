/**
 * @File         MModelConverter
 * 
 * @Created      2019-08-02 11:35:22
 *
 * @Author       DoubleYe
 *
 * MModelResource -> MModelConverter 2020-06-14 00:17:49
**/

#pragma once

#include "Utility/MGlobal.h"
#include "Resource/MResource.h"
#include "Utility/MString.h"
#include "Render/MVertex.h"
#include "Render/MMesh.h"
#include "Resource/MSkeletonResource.h"
#include "Resource/MMaterialResource.h"

struct aiNode;
struct aiScene;
struct aiMesh;

MORTY_SPACE_BEGIN

class MSkeletalAnimationResource;
class MBone;
class MScene;
class MEntity;
class MSkeleton;
class MMeshResource;
class MModelInstance;
class MTextureResource;
class MSkeletalAnimation;

enum class MModelConvertMaterialType
{
	E_Default_Forward = 0,
	E_PBR_Deferred
};

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
	virtual void PostProcess(MMaterial* pMaterial) = 0;
};

struct MORTY_API MModelConvertInfo
{
	MString strResourcePath;
	MString strOutputDir;
	MString strOutputName;
	
	bool bImportCamera = false;
	bool bImportLights = true;
	MModelConvertMaterialType eMaterialType;

	std::shared_ptr<MITextureDelegate> pTextureDelegate = nullptr;
	std::shared_ptr<MIMaterialDelegate> pMaterialDelegate = nullptr;
};

class MORTY_API MModelConverter
{
public:
	MModelConverter(MEngine* pEngine);
    virtual ~MModelConverter();

	bool Convert(const MModelConvertInfo& convertInfo);

protected:

	bool Load(const MString& strResourcePath);

	void ProcessNode(aiNode* pNode, const aiScene* pScene);
	void ProcessMeshVertices(aiMesh* pMesh, MMesh<MVertex>* pMMesh);
	void ProcessMeshVertices(aiMesh* pMesh, MMesh<MVertexWithBones>* pMMesh);
	void ProcessMeshIndices(aiMesh* pMesh, MIMesh* pMMesh);

	void BindVertexAndBones(MSkeleton* pSkeleton, aiMesh* pMesh, MMesh<MVertexWithBones>* pMMesh);

	void ProcessBones(const aiScene* pScene);
	void RecordBones(MSkeleton* pSkeleton, aiNode* pNode, const aiScene* pScene);
	void BindBones(MSkeleton* pSkeleton, aiNode* pNode, const aiScene* pScene, MBone* pParent = nullptr);

	void ProcessLights(const aiScene* pScene);

	void ProcessCameras(const aiScene* pScene);

	void ProcessAnimation(const aiScene* pScene);

	void ProcessMaterial(const aiScene* pScene, const uint32_t& nMaterialIdx);

	void ProcessTexture(const aiScene* pScene);

	MEntity* GetEntityFromNode(const aiScene* pScene, aiNode* pNode);

	std::shared_ptr<MMaterialResource> GetMaterial(const aiScene* pScene, const uint32_t& nMaterialIdx);

	MEngine* GetEngine() { return m_pEngine; }

private:

	MEngine* m_pEngine;
	MScene* m_pScene;

	MString m_strResourcePath;

	std::vector<std::pair<MString, std::shared_ptr<MMeshResource>>> m_vMeshes;
	std::vector<std::shared_ptr<MMaterialResource>> m_vMaterials;
	std::map<MString, std::shared_ptr<MTextureResource>> m_tRawTextures;
	std::set<std::shared_ptr<MResource>> m_tFileTextures;

	std::map<aiNode*, MEntity*> m_tNodeMaps;

	std::shared_ptr<MSkeletonResource> m_pSkeletonResource;
	MEntity* m_pModelEntity;;

	std::vector<std::shared_ptr<MSkeletalAnimationResource>> m_vSkeletalAnimation;


	bool bImportCamera = false;
	bool bImportLights = true;
	MModelConvertMaterialType eMaterialType;


	std::shared_ptr<MITextureDelegate> m_pTextureDelegate = nullptr;
	std::shared_ptr<MIMaterialDelegate> m_pMaterialDelegate = nullptr;

};

MORTY_SPACE_END