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
class MIMesh;
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

	enum class MECoordHoverType
	{
		None = 0,
		X = 1,
		Y = 2,
		Z = 3,
	};

	enum class MECoordMoveType
	{
		X = 1,
		Y = 2,
		Z = 4,
	};

	MTransformCoord3D();
	virtual ~MTransformCoord3D();

	void SetTarget3DNode(MNode* pNode);


	bool Input(MInputEvent* pEvent, MIViewport* pViewport);
	void Render(MIRenderer* pRenderer, MIViewport* pViewport);

protected:

private:

	M3DNode* m_pTargetNode;
	MECoordHoverType m_eCoordHoverType;
	unsigned int m_eCoordMoveType;

	MIMesh* m_pCoordRenderCache;
};


#endif
