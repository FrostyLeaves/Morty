/**
 * @File         MTransformCoord
 * 
 * @Created      2019-11-09 21:25:24
 *
 * @Author       Morty
**/

#ifndef _M_MTRANSFORMCOORD_H_
#define _M_MTRANSFORMCOORD_H_
#include "MGlobal.h"
#include "MObject.h"

class MNode;
class M3DNode;
class MIRenderer;
class MIViewport;
class MInputEvent;
class MORTY_CLASS MITransformCoord
{
public:
	MITransformCoord() {}
	virtual ~MITransformCoord() {}

public:

};


class MORTY_CLASS MTransformCoord3D : public MITransformCoord, public MObject
{
public:

	MTransformCoord3D();
	virtual ~MTransformCoord3D();

	void SetTarget3DNode(MNode* pNode);


	bool Input(MInputEvent* pEvent, MIViewport* pViewport);
	void Render(MIRenderer* pRenderer, MIViewport* pViewport);

private:

	M3DNode* m_pTargetNode;
};


#endif
