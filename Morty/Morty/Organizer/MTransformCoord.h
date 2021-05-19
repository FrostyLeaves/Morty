/**
 * @File         MTransformCoord
 * 
 * @Created      2019-11-09 21:25:24
 *
 * @Author       DoubleYe
**/

#ifndef _M_MTRANSFORMCOORD_H_
#define _M_MTRANSFORMCOORD_H_
#include "MGlobal.h"
#include "MObject.h"
#include "Vector.h"
#include "Type/MColor.h"

#include "MMesh.h"
#include "MPainter.h"

class MNode;
class MIMesh;
class MViewport;
class MIRenderer;
class MInputEvent;
class MRenderCommand;
class MORTY_API MITransformCoord
{
public:
	MITransformCoord() {}
	virtual ~MITransformCoord() {}

public:

};

class MORTY_API MTransformCoord3D : public MITransformCoord, public MObject
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

	void FillMesh2D(MViewport* pViewport, MMesh<MPainterVertex>& mesh);
	void FillMesh3D(MViewport* pViewport, MMesh<MPainterVertex>& mesh);

protected:

	void GetTranslationShapes(class MPainter2DLine* lines, class MPainter2DShape* rects, bool* vValid, int* vOrder, MViewport* pViewport);

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

};


#endif
