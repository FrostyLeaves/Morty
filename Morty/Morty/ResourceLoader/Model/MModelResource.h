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

class aiNode;
class aiScene;
class aiMesh;
class MMesh;
class MModel;
class MORTY_CLASS MModelResource : public MResource
{
public:
    MModelResource();
    virtual ~MModelResource();

	MModel* GetModelTemplate() { return m_pModelTemplate; }

protected:

	virtual bool Load(const MString& strResourcePath) override;

	void ProcessNode(aiNode* pNode, const aiScene* pScene, MModel* pModel);
	void ProcessMesh(aiMesh* pMesh, const aiScene* pScene, MMesh* pMMesh);

private:
    
    MModel* m_pModelTemplate;

};


#endif
