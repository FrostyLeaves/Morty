/**
 * @File         MTransformCoord
 * 
 * @Created      2019-11-09 21:25:24
 *
 * @Author       Pobrecito
**/

#ifndef _M_MTRANSFORMCOORD_H_
#define _M_MTRANSFORMCOORD_H_
#include "MGlobal.h"
#include "MObject.h"
#include "Vector.h"
#include "Type/MColor.h"

class MNode;
class M3DNode;
class MIRenderer;
class MViewport;
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
		XY = 3,
		Z = 4,
		XZ = 5,
		YZ = 6,
		XYZ = 7
	};

	MTransformCoord3D();
	virtual ~MTransformCoord3D();

	void SetTarget3DNode(MNode* pNode);

	bool Input(MInputEvent* pEvent, MViewport* pViewport);
	void Render(MIRenderer* pRenderer, MViewport* pViewport);

protected:

	void GetTranslationShapes(class MPainter2DLine* lines, class MPainter2DRect* rects, bool* vValid, int* vOrder, MViewport* pViewport);

	uint32_t GetAxisIndex(const MECoordHoverType& eType);
	
private:

	static const Vector3 m_vDirection[3];
	static const MColor m_vColor[3];
	Vector3 m_v3NormalizedDirection;
	Vector3 m_v3PlaneHitPoint;
	Vector3 m_v3TransformOrigin;
	Vector3 m_v3PlaneNormal;

	MObjectID m_unSelectedID;
	MECoordHoverType m_eCoordHoverType;
	MECoordHoverType m_eCoordMoveType;

	MIMesh* m_pCoordRenderCache;
};


#endif
