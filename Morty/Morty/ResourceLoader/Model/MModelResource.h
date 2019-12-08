/**
 * @File         MResourceModel
 * 
 * @Created      2019-08-02 11:35:22
 *
 * @Author       Morty
**/

#ifndef _M_MODELMRESOURCE_H_
#define _M_MODELMRESOURCE_H_
#include "MGlobal.h"
#include "MResource.h"
#include "MString.h"
#include "MVertex.h"
#include "MMesh.h"

struct aiNode;
struct aiScene;
struct aiMesh;
class MModel;
class MBone;
class MBoundsOBB;
class MSkeleton;
class MORTY_CLASS MModelResource : public MResource
{
public:
	enum MEModelVertexType {
		Normal = 0,
		Skeleton = 1,
	};
public:
    MModelResource();
    virtual ~MModelResource();

	const std::vector<MIMesh*>* GetMeshes() { return &m_vMeshes; };

	MBoundsOBB* GetOBB();

protected:

	virtual bool Load(const MString& strResourcePath) override;

	void ProcessNode(aiNode* pNode, const aiScene* pScene);
	void ProcessMeshVertices(aiMesh* pMesh, const aiScene* pScene, MMesh<MVertex>* pMMesh);
	void ProcessMeshVertices(aiMesh* pMesh, const aiScene* pScene, MMesh<MVertexWithBones>* pMMesh);
	void ProcessMeshIndices(aiMesh* pMesh, const aiScene* pScene, MIMesh* pMMesh);

	void RecordBones(aiMesh* pMesh, const aiScene* pScene, MMesh<MVertexWithBones>* pMMesh);
	void BindBones(aiNode* pNode, const aiScene* pScene, MBone* pParent = nullptr);

	void ProcessAnimation(const aiScene* pScene);
private:
    
	std::vector<MIMesh*> m_vMeshes;
	std::vector<MEModelVertexType> m_vVertexTypes;
	MBoundsOBB* m_pBoundsOBB;
	MSkeleton* m_pSkeleton;
};

#endif
