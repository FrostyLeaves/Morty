/**
 * @File         MPainter
 * 
 * @Created      2019-11-04 22:49:27
 *
 * @Author       Morty
**/

#ifndef _M_MPAINTER_H_
#define _M_MPAINTER_H_
#include "MGlobal.h"
#include "MObject.h"
#include "Vector.h"
#include "MType.h"
#include "MMesh.h"
#include <vector>

class MIViewport;

struct MPainterVertex
{
	Vector2 pos;
	Vector4 color;
};

class MORTY_CLASS MIPainterShape
{
public:
	MIPainterShape() {}
	virtual ~MIPainterShape() {}

	virtual unsigned int GetVertexCount() = 0;
	virtual unsigned int GetIndexCount() = 0;

	virtual bool FillData(MIViewport* pViewport, MMesh<MPainterVertex>& mesh) = 0;
	virtual bool TouchTest(const Vector2& pos, MIViewport* pViewport) { return false; }
};

class MORTY_CLASS MPainter2DLine : public MIPainterShape
{
public:
	MPainter2DLine(): m_v3Begin(), m_v3End(), m_lineColor(), m_fThickness(1.0f) {}
	MPainter2DLine(const Vector3& v3Begin, const Vector3& v3End, const MColor& lineColor,const float& fThickness = 1.0f)
		: m_v3Begin(v3Begin), m_v3End(v3End), m_lineColor(lineColor), m_fThickness(fThickness) {}

	virtual ~MPainter2DLine() {}

	virtual unsigned int GetVertexCount() override { return 4; }
	virtual unsigned int GetIndexCount() override { return 6; }

	virtual bool FillData(MIViewport* pViewport, MMesh<MPainterVertex>& mesh) override;
	virtual bool TouchTest(const Vector2& pos, MIViewport* pViewport) override;

	Vector2 GetDirection2D(MIViewport* pViewport);
	float GetLength2D(MIViewport* pViewport);

public:
	Vector3 m_v3Begin;
	Vector3 m_v3End;
	MColor m_lineColor;
	float m_fThickness;
};

#endif
