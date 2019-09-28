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
class MORTY_CLASS MModelResource : public MResource
{
public:
    MModelResource();
    virtual ~MModelResource();

	std::vector<MIMesh*>& GetMeshes() { return m_vMeshes; };

protected:

	virtual bool Load(const MString& strResourcePath) override;

	void ProcessNode(aiNode* pNode, const aiScene* pScene);
	void ProcessMesh(aiMesh* pMesh, const aiScene* pScene, MMesh<MVertex>* pMMesh);

private:
    
	std::vector<MIMesh*> m_vMeshes;

};


#endif
