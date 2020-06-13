/**
 * @File         MResourceModel
 * 
 * @Created      2019-08-02 11:35:22
 *
 * @Author       Pobrecito
**/

#ifndef _M_MODELMRESOURCE_H_
#define _M_MODELMRESOURCE_H_
#include "MGlobal.h"
#include "MResource.h"
#include "MString.h"
#include "MVertex.h"
#include "MMesh.h"

#include <map>

struct aiNode;
struct aiScene;
struct aiMesh;
class MBone;
class MModel;
class MSkeleton;
class MSkeletonResource;
class MMeshResource;
class MSkeletalAnimation;
class MORTY_CLASS MModelResource : public MResource
{
public:
	M_RESOURCE(MModelResource)
    MModelResource();
    virtual ~MModelResource();

	MSkeleton* GetSkeleton() { return m_pSkeleton; }
	const std::vector<MMeshResource*>* GetMeshes() { return &m_vMeshes; }
	const std::map<MString, MSkeletalAnimation*>* GetAnimations() { return &m_tSkeletalAnimation; }
	const std::vector<MString>* GetAnimationsName() { return &m_vSkeletalAnimation; }


public:

	virtual void OnDelete() override;

protected:

	virtual bool Load(const MString& strResourcePath) override;

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
    
	std::vector<MMeshResource*> m_vMeshes;

	MSkeleton* m_pSkeleton;
	MSkeletonResource* m_pSkeletonResource;

	std::vector<MString> m_vSkeletalAnimation;
	std::map<MString, MSkeletalAnimation*> m_tSkeletalAnimation;
};

#endif
