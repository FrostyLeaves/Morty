/**
 * @File         MNodeResource
 * 
 * @Created      2020-05-30 23:16:16
 *
 * @Author       DoubleYe
**/

#ifndef _M_MNODERESOURCE_H_
#define _M_MNODERESOURCE_H_
#include "MGlobal.h"
#include "MResource.h"

class MNode;
class MORTY_API MNodeResource : public MResource
{
public:
	M_RESOURCE(MNodeResource);
    MNodeResource();
    virtual ~MNodeResource();


public:

    MNode* CreateNode();
    void SaveByNode(MNode* pNode);

protected:
    
	virtual bool Load(const MString& strResourcePath) override;

private:

};

#endif
