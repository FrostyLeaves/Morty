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
	enum MEModelType
	{
		MStaticModel = 1,
		MAnimationModel = 2,
	};
public:
    MModelResource();
    virtual ~MModelResource();

	std::vector<MMesh<MVertex>*>& GetMeshes() { return m_vMeshes; };

	MBoundsOBB* GetOBB();

protected:

	virtual bool Load(const MString& strResourcePath) override;

	void ProcessNode(aiNode* pNode, const aiScene* pScene);
	void ProcessMesh(aiMesh* pMesh, const aiScene* pScene, MMesh<MVertex>* pMMesh);

	void RecordBones(aiMesh* pMesh, const aiScene* pScene);
	void BindBones(aiNode* pNode, const aiScene* pScene, MBone* pParent = nullptr);

	void ProcessAnimation(const aiScene* pScene);
private:
    
	std::vector<MMesh<MVertex>*> m_vMeshes;
	MBoundsOBB* m_pBoundsOBB;
	MSkeleton* m_pSkeleton;
};


#endif
