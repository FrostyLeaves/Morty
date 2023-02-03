/**
 * @File         MModelConverter
 * 
 * @Created      2019-08-02 11:35:22
 *
 * @Author       DoubleYe
 *
 * MModelResource -> MModelConverter 2020-06-14 00:17:49
**/

#ifndef _M_MODELCONVERTER_H_
#define _M_MODELCONVERTER_H_
#include "Utility/MGlobal.h"
#include "Resource/MResource.h"
#include "Utility/MString.h"
#include "Render/MVertex.h"
#include "Render/MMesh.h"
#include "Resource/MSkeletonResource.h"

#include <map>

struct aiNode;
struct aiScene;
struct aiMesh;
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

struct MORTY_API MModelConvertInfo
{
	MString strResourcePath;
	MString strOutputDir;
	MString strOutputName;
	
	MModelConvertMaterialType eMaterialType;
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
	void ProcessMeshVertices(aiMesh* pMesh, const aiScene* pScene, MMesh<MVertex>* pMMesh);
	void ProcessMeshVertices(aiMesh* pMesh, const aiScene* pScene, MMesh<MVertexWithBones>* pMMesh);
	void ProcessMeshIndices(aiMesh* pMesh, const aiScene* pScene, MIMesh* pMMesh);

	void BindVertexAndBones(aiMesh* pMesh, const aiScene* pScene, MMesh<MVertexWithBones>* pMMesh);

	void ProcessBones(const aiScene* pScene);
	void RecordBones(aiNode* pNode, const aiScene* pScene);
	void BindBones(aiNode* pNode, const aiScene* pScene, MBone* pParent = nullptr);

	void ProcessLights(const aiScene* pScene);

	void ProcessCameras(const aiScene* pScene);

	void ProcessAnimation(const aiScene* pScene);

	void ProcessMaterial(const aiScene* pScene, const uint32_t& nMaterialIdx);

	void ProcessTexture(const aiScene* pScene);

	MEntity* GetEntityFromNode(const aiScene* pScene, aiNode* pNode);

	std::shared_ptr<MMaterial> GetMaterial(const aiScene* pScene, const uint32_t& nMaterialIdx);

	MEngine* GetEngine() { return m_pEngine; }

private:

	MEngine* m_pEngine;
	MScene* m_pScene;

	MString m_strResourcePath;

	std::vector<std::pair<MString, std::shared_ptr<MMeshResource>>> m_vMeshes;
	std::vector<std::shared_ptr<MMaterial>> m_vMaterials;
	std::map < MString, std::shared_ptr<MTextureResource>> m_tTextures;

	std::map<aiNode*, MEntity*> m_tNodeMaps;

	std::shared_ptr<MSkeletonResource> m_pSkeleton;
	MEntity* m_pModelEntity;;

	std::vector<std::shared_ptr<MSkeletalAnimation>> m_vSkeletalAnimation;

	MModelConvertMaterialType eMaterialType;

};

#endif
