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
class MViewport;
class MORTY_API MInputNode : public MNode
{
public:
	M_OBJECT(MInputNode);
    MInputNode();
    virtual ~MInputNode();

	typedef std::function<bool(MInputEvent*, MViewport*)> MInputCallback;

public:

	void SetInputCallback(const MInputCallback& func) { m_funcInputCallback = func; }

	virtual bool Input(MInputEvent* pEvent, MViewport* pViewport);

private:

	MInputCallback m_funcInputCallback;
};

#endif
