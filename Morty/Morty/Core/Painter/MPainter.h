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
	Vector3 pos;
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
	MPainter2DLine(): m_v2Begin(), m_v2End(), m_lineColor(), m_fThickness(1.0f) {}
	MPainter2DLine(const Vector2& v2Begin, const Vector2& v2End, const MColor& lineColor,const float& fThickness = 1.0f) : MIPainterShape()
		, m_v2Begin(v2Begin), m_v2End(v2End), m_lineColor(lineColor), m_fThickness(fThickness) {}

	virtual ~MPainter2DLine() {}

	virtual unsigned int GetVertexCount() override { return 4; }
	virtual unsigned int GetIndexCount() override { return 6; }

	virtual bool FillData(MIViewport* pViewport, MMesh<MPainterVertex>& mesh) override;
	virtual bool TouchTest(const Vector2& pos, MIViewport* pViewport) override;

	Vector2 GetDirection2D(MIViewport* pViewport);
	float GetLength2D(MIViewport* pViewport);

public:
	Vector2 m_v2Begin;
	Vector2 m_v2End;
	MColor m_lineColor;
	float m_fThickness;
};

class MORTY_CLASS MPainter2DRect : public MIPainterShape
{
public:
	MPainter2DRect() : m_vPoint(), m_rectColor() {}
	MPainter2DRect(const Vector2& point0, const Vector2& point1, const Vector2& point2, const Vector2& point3, const MColor& color) : MIPainterShape()
		, m_vPoint{ point0, point1, point2, point3 }, m_rectColor(color) {}

	virtual ~MPainter2DRect() {}

	virtual unsigned int GetVertexCount() override { return 4; }
	virtual unsigned int GetIndexCount() override { return 6; }

	virtual bool FillData(MIViewport* pViewport, MMesh<MPainterVertex>& mesh) override;
	virtual bool TouchTest(const Vector2& pos, MIViewport* pViewport) override;


public:
	Vector2 m_vPoint[4];
	MColor m_rectColor;
};

class MORTY_CLASS MPainter2DLine3D : public MIPainterShape
{
public:
	MPainter2DLine3D() : m_v3Begin(), m_v3End(), m_lineColor(), m_fThickness(1.0f) {}
	MPainter2DLine3D(const Vector3& v3Begin, const Vector3& v3End, const MColor& color, const float& fTickness) : MIPainterShape()
		, m_v3Begin(v3Begin), m_v3End(v3End), m_lineColor(color), m_fThickness(fTickness) {}

	virtual ~MPainter2DLine3D() {}

	virtual unsigned int GetVertexCount() override { return 4; }
	virtual unsigned int GetIndexCount() override { return 6; }

	virtual bool FillData(MIViewport* pViewport, MMesh<MPainterVertex>& mesh) override;
	virtual bool TouchTest(const Vector2& pos, MIViewport* pViewport) override;


public:

	Vector3 m_v3Begin;
	Vector3 m_v3End;

	MColor m_lineColor;
	float m_fThickness;
};

#endif
