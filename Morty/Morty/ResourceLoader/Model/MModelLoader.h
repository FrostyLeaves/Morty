/**
 * @File         MModelLoader
 * 
 * @Created      2019-08-06 16:51:51
 *
 * @Author       Morty
**/

#ifndef _M_MMODELLOADER_H_
#define _M_MMODELLOADER_H_
#include "MGlobal.h"

#include "MResourceLoader.h"

class aiNode;
class aiScene;
class aiMesh;
class MMesh;
class MModel;
class MORTY_CLASS MModelLoader : public MResourceLoader
{
public:
    MModelLoader();
    virtual ~MModelLoader();

public:
    
    virtual MResource* Load(const char* svPath);

protected:
    void ProcessNode(aiNode* pNode, const aiScene* pScene, MModel* pModel);
    
    void ProcessMesh(aiMesh* pMesh, const aiScene* pScene, MMesh* pMMesh);
    
private:

};


#endif
