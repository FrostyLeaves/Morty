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
	MPainter2DRect() :m_v3Center(), m_v3Normal(), m_v3Up(), m_rectColor(), m_fWidth(), m_fHeight() {}
	MPainter2DRect(const Vector3& v3Center, const Vector3& v3Normal, const Vector3& v3Up, const MColor& rectColor, const float& fWidth, const float& fHeight);

	virtual ~MPainter2DRect() {}

	virtual unsigned int GetVertexCount() override { return 4; }
	virtual unsigned int GetIndexCount() override { return 6; }

	virtual bool FillData(MIViewport* pViewport, MMesh<MPainterVertex>& mesh) override;
	virtual bool TouchTest(const Vector2& pos, MIViewport* pViewport) override;

public:
	Vector3 m_v3Center;
	Vector3 m_v3Normal;
	Vector3 m_v3Up;
	MColor m_rectColor;
	float m_fWidth;
	float m_fHeight;
};

#endif
