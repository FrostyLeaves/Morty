/**
 * @File         MInputNode
 * 
 * @Created      2019-12-27 19:56:48
 *
 * @Author       Pobrecito
**/

#ifndef _M_MINPUTNODE_H_
#define _M_MINPUTNODE_H_
#include "MGlobal.h"
#include "MNode.h"

class MInputEvent;
class MIViewport;
class MORTY_CLASS MInputNode : public MNode
{
public:
	M_OBJECT(MInputNode);
    MInputNode();
    virtual ~MInputNode();

	typedef std::function<bool(MInputEvent*, MIViewport*)> MInputCallback;

public:

	void SetInputCallback(const MInputCallback& func) { m_funcInputCallback = func; }

	virtual bool Input(MInputEvent* pEvent, MIViewport* pViewport);

private:

	MInputCallback m_funcInputCallback;
};

#endif
