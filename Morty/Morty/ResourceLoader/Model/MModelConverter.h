/**
 * @File         MModelConverter
 * 
 * @Created      2019-08-02 11:35:22
 *
 * @Author       Pobrecito
 *
 * MModelResource -> MModelConverter 2020-06-14 00:17:49
**/

#ifndef _M_MODELCONVERTER_H_
#define _M_MODELCONVERTER_H_
#include "MGlobal.h"
#include "MResource.h"
#include "MString.h"
#include "MVertex.h"
#include "MMesh.h"
#include "MSkeletonResource.h"

#include <map>

struct aiNode;
struct aiScene;
struct aiMesh;
class MBone;
class MModel;
class MSkeleton;
class MMeshResource;
class MSkeletalAnimation;
class MORTY_CLASS MModelConverter
{
public:
	MModelConverter(MEngine* pEngine);
    virtual ~MModelConverter();

	bool Convert(const MString& strResourcePath, const MString& strOutputDir, const MString& strOutputName);


protected:

	bool Load(const MString& strResourcePath);

	void ProcessNode(aiNode* pNode, const aiScene* pScene, std::vector<unsigned int>& vMaterialIndices, const Matrix4& matRotation);
	void ProcessMeshVertices(aiMesh* pMesh, const aiScene* pScene, MMesh<MVertex>* pMMesh, const Matrix4& matRotation);
	void ProcessMeshVertices(aiMesh* pMesh, const aiScene* pScene, MMesh<MVertexWithBones>* pMMesh, const Matrix4& matRotation);
	void ProcessMeshIndices(aiMesh* pMesh, const aiScene* pScene, MIMesh* pMMesh);

	void BindVertexAndBones(aiMesh* pMesh, const aiScene* pScene, MMesh<MVertexWithBones>* pMMesh);

	void ProcessBones(const aiScene* pScene);
	void RecordBones(aiNode* pNode, const aiScene* pScene);
	void BindBones(aiNode* pNode, const aiScene* pScene, MBone* pParent = nullptr);

	void ProcessAnimation(const aiScene* pScene);

	void ProcessMaterial(const aiScene* pScene, std::vector<unsigned int>& vMaterialIndices);

private:

	MEngine* m_pEngine;

	MString m_strResourcePath;
    
	std::vector<MMeshResource*> m_vMeshes;
	MSkeletonResource* m_pSkeleton;

	std::vector<MSkeletalAnimation*> m_vSkeletalAnimation;
};

#endif
