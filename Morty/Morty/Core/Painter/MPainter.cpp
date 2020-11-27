#include "MPainter.h"
#include "MMesh.h"
#include "MViewport.h"
#include "MCamera.h"
#include <algorithm>

Vector2 MPainter2DLine::GetDirection2D(MViewport* pViewport)
{
	Vector2 dir = m_v2End - m_v2Begin;
	dir.Normalize();

	return dir;
}

float MPainter2DLine::GetLength2D(MViewport* pViewport)
{
	Vector3 v2Begin2D = m_v2Begin;
	Vector3 v2End2D = m_v2End;

	v2Begin2D = (v2Begin2D + Vector2(1.0, 1.0)) * 0.5;
	v2Begin2D.x *= pViewport->GetWidth();
	v2Begin2D.y *= pViewport->GetHeight();
	v2Begin2D.z = 0;

	v2End2D = (v2End2D + Vector2(1.0, 1.0)) * 0.5;
	v2End2D.x *= pViewport->GetWidth();
	v2End2D.y *= pViewport->GetHeight();
	v2End2D.z = 0;

	return (v2End2D - v2Begin2D).Length();
}

bool MPainter2DLine::FillData(MViewport* pViewport, MMesh<MPainterVertex>& mesh)
{
	if (nullptr == pViewport)
		return false;

	Vector2 v2Begin2D = m_v2Begin;
	Vector2 v2End2D = m_v2End;

	if (v2Begin2D.x > v2End2D.x || (v2Begin2D.x == v2End2D.x && v2Begin2D.y < v2End2D.y))
	{
		Vector2 temp = v2Begin2D;
		v2Begin2D = v2End2D;
		v2End2D = temp;
	}

	Vector2 v2Normal = v2End2D - v2Begin2D;
	if (fabs(v2Normal.x) < 0.0001)
		v2Normal = Vector2(1, 0);
	else
	{
		v2Normal = Vector2(-v2Normal.y / v2Normal.x, 1);
		v2Normal.Normalize();
	}

	uint32_t unVerticesLength = mesh.GetVerticesLength();
	uint32_t unIndicesLength = mesh.GetIndicesLength();

	mesh.ResizeVertices(unVerticesLength + GetVertexCount());
	mesh.ResizeIndices(unIndicesLength + GetIndexCount(), 1);

	mesh.GetVertices()[unVerticesLength + 0].pos.x = v2Begin2D.x + v2Normal.x * -m_fThickness / pViewport->GetSize().x;
	mesh.GetVertices()[unVerticesLength + 0].pos.y = v2Begin2D.y + v2Normal.y * -m_fThickness / pViewport->GetSize().y;
	mesh.GetVertices()[unVerticesLength + 0].color = m_lineColor.ToVector4();

	mesh.GetVertices()[unVerticesLength + 1].pos.x = v2Begin2D.x + v2Normal.x * m_fThickness / pViewport->GetSize().x;
	mesh.GetVertices()[unVerticesLength + 1].pos.y = v2Begin2D.y + v2Normal.y * m_fThickness / pViewport->GetSize().y;
	mesh.GetVertices()[unVerticesLength + 1].color = m_lineColor.ToVector4();

	mesh.GetVertices()[unVerticesLength + 2].pos.x = v2End2D.x + v2Normal.x * -m_fThickness / pViewport->GetSize().x;
	mesh.GetVertices()[unVerticesLength + 2].pos.y = v2End2D.y + v2Normal.y * -m_fThickness / pViewport->GetSize().y;
	mesh.GetVertices()[unVerticesLength + 2].color = m_lineColor.ToVector4();

	mesh.GetVertices()[unVerticesLength + 3].pos.x = v2End2D.x + v2Normal.x * m_fThickness / pViewport->GetSize().x;
	mesh.GetVertices()[unVerticesLength + 3].pos.y = v2End2D.y + v2Normal.y * m_fThickness / pViewport->GetSize().y;
	mesh.GetVertices()[unVerticesLength + 3].color = m_lineColor.ToVector4();

	static uint32_t const indices[] = {
		0, 1, 2,
		1, 3, 2
	};

	for (uint32_t i = 0; i < 6; ++i)
		mesh.GetIndices()[unIndicesLength + i] = unVerticesLength + indices[i];

	mesh.SetNeedUpload();
	return true;
}

bool MPainter2DLine::TouchTest(const Vector2& pos, MViewport* pViewport)
{
	if (nullptr == pViewport)
		return false;

	float fHalfThickness = m_fThickness * 0.5f;

	Vector2 v2Begin2D = m_v2Begin;
	Vector2 v2End2D = m_v2End;

	v2Begin2D = (v2Begin2D + Vector2(1.0, 1.0)) * 0.5;
	v2Begin2D.x *= pViewport->GetWidth();
	v2Begin2D.y *= pViewport->GetHeight();

	v2End2D = (v2End2D + Vector2(1.0, 1.0)) * 0.5;
	v2End2D.x *= pViewport->GetWidth();
	v2End2D.y *= pViewport->GetHeight();

	float fValue = ((v2End2D.x - v2Begin2D.x) * (pos.x - v2Begin2D.x) + (v2End2D.y - v2Begin2D.y) * (pos.y - v2Begin2D.y)) / ((v2End2D.x - v2Begin2D.x) * (v2End2D.x - v2Begin2D.x) + (v2End2D.y - v2Begin2D.y) * (v2End2D.y - v2Begin2D.y));

	if (0.0f > fValue || fValue > 1.0f)
		return false;

	Vector2 v2Projection = v2Begin2D + (v2End2D - v2Begin2D) * fValue;
	if ((pos - v2Projection).Length() <= fHalfThickness)
		return true;

	return false;
}

MPainter2DRect::MPainter2DRect(const Vector2& point0, const Vector2& point1, const Vector2& point2, const Vector2& point3, const MColor& color) : MIPainterShape()
, m_rectColor(color)
{
	m_vPoint[0] = point0;
	m_vPoint[1] = point1;
	m_vPoint[2] = point2;
	m_vPoint[3] = point3;
}

bool MPainter2DRect::FillData(MViewport* pViewport, MMesh<MPainterVertex>& mesh)
{
	if (nullptr == pViewport)
		return false;

	uint32_t unVerticesLength = mesh.GetVerticesLength();
	uint32_t unIndicesLength = mesh.GetIndicesLength();

	mesh.ResizeVertices(unVerticesLength + GetVertexCount());
	mesh.ResizeIndices(unIndicesLength + GetIndexCount(), 1);

	Vector4 color = m_rectColor.ToVector4();
	for (int i = 0; i < 4; ++i)
	{
		mesh.GetVertices()[unVerticesLength + i].pos = m_vPoint[i];
		mesh.GetVertices()[unVerticesLength + i].color = color;
	}

	static uint32_t const indices[] = {
		0, 1, 2,
		0, 2, 3,
	};

	for (uint32_t i = 0; i < 6; ++i)
		mesh.GetIndices()[unIndicesLength + i] = unVerticesLength + indices[i];
	

	mesh.SetNeedUpload();
	return true;
}

bool MPainter2DRect::TouchTest(const Vector2& pos, MViewport* pViewport)
{
	Vector2 normalPos = pos;
	normalPos.x /= pViewport->GetWidth();
	normalPos.y /= pViewport->GetHeight();
	normalPos = normalPos * 2 - Vector2(1.0, 1.0);

	float sn01 = (m_vPoint[0] - normalPos).CrossProduct(m_vPoint[1] - normalPos);
	float sn12 = (m_vPoint[1] - normalPos).CrossProduct(m_vPoint[2] - normalPos);
	float sn23 = (m_vPoint[2] - normalPos).CrossProduct(m_vPoint[3] - normalPos);
	float sn30 = (m_vPoint[3] - normalPos).CrossProduct(m_vPoint[0] - normalPos);

	if ((sn01 * sn12) > 0 && (sn01 * sn23) > 0 && (sn01 * sn30) > 0)
		return true;

// 	if (fabsf(sn01 * sn12 * sn23 * sn30) < 1e-6)
// 		return true;

	return false;
}

bool MPainter3DLine::FillData(MViewport* pViewport, MMesh<MPainterVertex>& mesh)
{
	if (nullptr == pViewport)
		return false;

	Vector3 v3CameraWorldPosition = pViewport->GetCamera()->GetWorldPosition();
	Vector3 v3Center = (m_v3Begin + m_v3End) * 0.5;
	Vector3 v3Normal = (v3CameraWorldPosition - v3Center).CrossProduct(m_v3End - m_v3Begin);
	v3Normal.Normalize();

	Vector3 v2Center, v2Nab;
	if (false == pViewport->ConvertWorldPointToViewport(v3Center, v2Center))
		return false;
	if (false == pViewport->ConvertWorldPointToViewport(v3Center + v3Normal, v2Nab))
		return false;
	
	Vector3 v3Thick3D = v3Normal * (m_fThickness / (v2Nab - v2Center).Length());


	uint32_t unVerticesLength = mesh.GetVerticesLength();
	uint32_t unIndicesLength = mesh.GetIndicesLength();

	mesh.ResizeVertices(unVerticesLength + GetVertexCount());
	mesh.ResizeIndices(unIndicesLength + GetIndexCount(), 1);

	mesh.GetVertices()[unVerticesLength + 0].pos = m_v3Begin - v3Thick3D;
	mesh.GetVertices()[unVerticesLength + 0].color = m_lineColor.ToVector4();

	mesh.GetVertices()[unVerticesLength + 1].pos = m_v3Begin + v3Thick3D;
	mesh.GetVertices()[unVerticesLength + 1].color = m_lineColor.ToVector4();

	mesh.GetVertices()[unVerticesLength + 2].pos = m_v3End + v3Thick3D;
	mesh.GetVertices()[unVerticesLength + 2].color = m_lineColor.ToVector4();

	mesh.GetVertices()[unVerticesLength + 3].pos = m_v3End - v3Thick3D;
	mesh.GetVertices()[unVerticesLength + 3].color = m_lineColor.ToVector4();

	static uint32_t const indices[] = {
		0, 2, 1,
		0, 3, 2
	};

	for (uint32_t i = 0; i < 6; ++i)
		mesh.GetIndices()[unIndicesLength + i] = unVerticesLength + indices[i];

	mesh.SetNeedUpload();
	return true;
}

bool MPainter3DLine::TouchTest(const Vector2& pos, MViewport* pViewport)
{
	return false;
}
