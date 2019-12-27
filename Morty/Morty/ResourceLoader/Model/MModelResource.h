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
class MModel;
class MBone;
class MBoundsOBB;
class MSkeleton;
class MSkeletalAnimation;
class MORTY_CLASS MModelResource : public MResource
{
public:
	enum MEMeshVertexType {
		Normal = 0,
		Skeleton = 1,
	};
public:
    MModelResource();
    virtual ~MModelResource();

	const std::vector<MIMesh*>* GetMeshes() { return &m_vMeshes; };
	const MSkeleton* GetSkeleton() { return m_pSkeleton; }
	const MBoundsOBB* GetOBB();
	const std::map<MString, MSkeletalAnimation*>* GetAnimations() { return &m_tSkeletalAnimation; }
	const std::vector<MString>* GetAnimationsName() { return &m_vSkeletalAnimation; }

	MEMeshVertexType GetMeshVertexType(const unsigned int& unIndex);
	MMaterial* GetMeshDefaultMaterial(const unsigned int& unIndex);

protected:

	virtual bool Load(const MString& strResourcePath) override;

	void ProcessNode(aiNode* pNode, const aiScene* pScene);
	void ProcessMeshVertices(aiMesh* pMesh, const aiScene* pScene, MMesh<MVertex>* pMMesh);
	void ProcessMeshVertices(aiMesh* pMesh, const aiScene* pScene, MMesh<MVertexWithBones>* pMMesh);
	void ProcessMeshIndices(aiMesh* pMesh, const aiScene* pScene, MIMesh* pMMesh);

	void BindVertexAndBones(aiMesh* pMesh, const aiScene* pScene, MMesh<MVertexWithBones>* pMMesh);

	void ProcessBones(const aiScene* pScene);
	void RecordBones(aiNode* pNode, const aiScene* pScene);
	void BindBones(aiNode* pNode, const aiScene* pScene, MBone* pParent = nullptr);

	void ProcessAnimation(const aiScene* pScene);

	void ProcessMaterial(const aiScene* pScene);

private:
    
	std::vector<MIMesh*> m_vMeshes;
	std::vector<MEMeshVertexType> m_vVertexTypes;

	std::vector<MMaterial*> m_vDefaultMaterial;

	MBoundsOBB* m_pBoundsOBB;
	MSkeleton* m_pSkeleton;

	std::vector<MString> m_vSkeletalAnimation;
	std::map<MString, MSkeletalAnimation*> m_tSkeletalAnimation;
};

#endif
