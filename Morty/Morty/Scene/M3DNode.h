/**
 * @File         M3DNode
 * 
 * @Created      2019-05-25 19:54:30
 *
 * @Author       Morty
**/

#ifndef _M_M3DNODE_H_
#define _M_M3DNODE_H_
#include "MGlobal.h"

#include "MNode.h"

#include "Matrix.h"

class MORTY_CLASS M3DNode : public MNode
{
public:
    M3DNode();
    virtual ~M3DNode();

public:

	virtual M3DNode* Create() override;

private:


	Matrix4 m_m4Transform;
};


#endif
