/**
 * @File         MPainter
 * 
 * @Created      2019-11-04 22:49:27
 *
 * @Author       DoubleYe
**/

#ifndef _M_MPAINTER_H_
#define _M_MPAINTER_H_
#include "MGlobal.h"
#include "MObject.h"
#include "Vector.h"
#include "Type/MType.h"
#include "MMesh.h"
#include <vector>

class MViewport;

struct MPainterVertex
{
	Vector3 pos;
	Vector4 color;
};

class MORTY_API MIPainterShape
{
public:
	MIPainterShape() {}
	virtual ~MIPainterShape() {}

	virtual uint32_t GetVertexCount() = 0;
	virtual uint32_t GetIndexCount() = 0;

	virtual bool FillData(MViewport* pViewport, MMesh<MPainterVertex>& mesh) = 0;
	virtual bool TouchTest(const Vector2& pos, MViewport* pViewport) { return false; }
};

class MORTY_API MPainter2DLine : public MIPainterShape
{
public:
	MPainter2DLine(): m_v2Begin(), m_v2End(), m_lineColor(), m_fThickness(1.0f) {}
	MPainter2DLine(const Vector2& v2Begin, const Vector2& v2End, const MColor& lineColor,const float& fThickness = 1.0f) : MIPainterShape()
		, m_v2Begin(v2Begin), m_v2End(v2End), m_lineColor(lineColor), m_fThickness(fThickness) {}

	virtual ~MPainter2DLine() {}

	virtual uint32_t GetVertexCount() override { return 4; }
	virtual uint32_t GetIndexCount() override { return 6; }

	virtual bool FillData(MViewport* pViewport, MMesh<MPainterVertex>& mesh) override;
	virtual bool TouchTest(const Vector2& pos, MViewport* pViewport) override;

	Vector2 GetDirection2D(MViewport* pViewport);
	float GetLength2D(MViewport* pViewport);

public:
	Vector2 m_v2Begin;
	Vector2 m_v2End;
	MColor m_lineColor;
	float m_fThickness;
};

class MORTY_API MPainter2DShape : public MIPainterShape
{
public:
	MPainter2DShape() : m_vPoint(), m_rectColor() {}
	MPainter2DShape(const std::vector<Vector2>& points, const MColor& color);

	virtual ~MPainter2DShape() {}

	virtual uint32_t GetVertexCount() override;
	virtual uint32_t GetIndexCount() override;

	virtual bool FillData(MViewport* pViewport, MMesh<MPainterVertex>& mesh) override;
	virtual bool TouchTest(const Vector2& pos, MViewport* pViewport) override;


public:
	std::vector<Vector2> m_vPoint;
	MColor m_rectColor;
};

class MORTY_API MPainter3DLine : public MIPainterShape
{
public:
	MPainter3DLine() : m_v3Begin(), m_v3End(), m_lineColor(), m_fThickness(1.0f) {}
	MPainter3DLine(const Vector3& v3Begin, const Vector3& v3End, const MColor& color, const float& fTickness) : MIPainterShape()
		, m_v3Begin(v3Begin), m_v3End(v3End), m_lineColor(color), m_fThickness(fTickness) {}

	virtual ~MPainter3DLine() {}

	virtual uint32_t GetVertexCount() override { return 4; }
	virtual uint32_t GetIndexCount() override { return 6; }

	virtual bool FillData(MViewport* pViewport, MMesh<MPainterVertex>& mesh) override;
	virtual bool TouchTest(const Vector2& pos, MViewport* pViewport) override;


public:

	Vector3 m_v3Begin;
	Vector3 m_v3End;

	MColor m_lineColor;
	float m_fThickness;
};

#endif
