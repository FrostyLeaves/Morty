/**
 * @File         MRenderTaskNode
 * 
 * @Created      2021-07-16 22:02:25
 *
 * @Author       Pobrecito
**/

#ifndef _M_MRENDERTASKNODE_H_
#define _M_MRENDERTASKNODE_H_
#include "MGlobal.h"

#include "MTaskNode.h"
#include "MRenderPass.h"

class MRenderTaskNodeOutput;
class MORTY_API MRenderTaskNode : public MTaskNode
{
	MORTY_CLASS(MRenderTaskNode)
public:
    MRenderTaskNode();
    virtual ~MRenderTaskNode();

public:

	MRenderTaskNodeOutput* AppendOutput();

	MRenderPass* GetRenderPass() { return &m_renderpass; }

public:
	virtual void OnCreated() override;
	virtual void OnCompile() override;
	virtual void OnDelete() override;

private:


    MRenderPass m_renderpass;
};


#endif
