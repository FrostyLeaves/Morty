#include "MPainter.h"
#include "MMesh.h"
#include "MIViewport.h"
#include "MCamera.h"


Vector2 MPainter2DLine::GetDirection2D(MIViewport* pViewport)
{
	Vector2 dir = m_v2End - m_v2Begin;
	dir.Normalize();

	return dir;
}

float MPainter2DLine::GetLength2D(MIViewport* pViewport)
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

bool MPainter2DLine::FillData(MIViewport* pViewport, MMesh<MPainterVertex>& mesh)
{
	if (nullptr == pViewport)
		return false;

	Vector2 v2Begin2D = m_v2Begin;
	Vector2 v2End2D = m_v2End;

	Vector2 v2Normal = v2End2D - v2Begin2D;
	if (fabs(v2Normal.x) < 0.0001)
		v2Normal = Vector2(1, 0);
	else
	{
		v2Normal = Vector2(-v2Normal.y / v2Normal.x, 1);
		v2Normal.Normalize();
	}

	mesh.CreateVertices(GetVertexCount());
	mesh.CreateIndices(GetIndexCount(), 1);

	mesh.GetVertices()[0].pos.x = v2Begin2D.x + v2Normal.x * -m_fThickness / pViewport->GetSize().x;
	mesh.GetVertices()[0].pos.y = v2Begin2D.y + v2Normal.y * -m_fThickness / pViewport->GetSize().y;
	mesh.GetVertices()[0].color = m_lineColor.ToVector4();

	mesh.GetVertices()[1].pos.x = v2Begin2D.x + v2Normal.x * m_fThickness / pViewport->GetSize().x;
	mesh.GetVertices()[1].pos.y = v2Begin2D.y + v2Normal.y * m_fThickness / pViewport->GetSize().y;
	mesh.GetVertices()[1].color = m_lineColor.ToVector4();

	mesh.GetVertices()[2].pos.x = v2End2D.x + v2Normal.x * -m_fThickness / pViewport->GetSize().x;
	mesh.GetVertices()[2].pos.y = v2End2D.y + v2Normal.y * -m_fThickness / pViewport->GetSize().y;
	mesh.GetVertices()[2].color = m_lineColor.ToVector4();

	mesh.GetVertices()[3].pos.x = v2End2D.x + v2Normal.x * m_fThickness / pViewport->GetSize().x;
	mesh.GetVertices()[3].pos.y = v2End2D.y + v2Normal.y * m_fThickness / pViewport->GetSize().y;
	mesh.GetVertices()[3].color = m_lineColor.ToVector4();

	static unsigned int const indices[] = {
		0, 1, 2,
		1, 3, 2
	};

	memcpy(mesh.GetIndices(), &indices, 2 * 3 * sizeof(unsigned int));

	mesh.SetNeedUpload();
	return true;
}

bool MPainter2DLine::TouchTest(const Vector2& pos, MIViewport* pViewport)
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

bool MPainter2DTriangle::FillData(MIViewport* pViewport, MMesh<MPainterVertex>& mesh)
{
	if (nullptr == pViewport)
		return false;

	mesh.CreateVertices(GetVertexCount());
	mesh.CreateIndices(GetIndexCount(), 1);

	Vector4 color = m_triangleColor.ToVector4();
	for (int i = 0; i < 3; ++i)
	{
		mesh.GetVertices()[i].pos = m_vPoint[i];
		mesh.GetVertices()[i].color = color;
	}

	static unsigned int const indices[] = {
		0, 1, 2
	};

	memcpy(mesh.GetIndices(), &indices, 1 * 3 * sizeof(unsigned int));

	mesh.SetNeedUpload();
	return true;
}

bool MPainter2DTriangle::TouchTest(const Vector2& pos, MIViewport* pViewport)
{
	Vector2 normalPos = pos;
	normalPos.x /= pViewport->GetWidth();
	normalPos.y /= pViewport->GetHeight();
	normalPos = normalPos * 2 - Vector2(1.0, 1.0);

	float sn01 = (m_vPoint[0] - normalPos).CrossProduct(m_vPoint[1] - normalPos);
	float sn12 = (m_vPoint[1] - normalPos).CrossProduct(m_vPoint[2] - normalPos);
	float sn20 = (m_vPoint[2] - normalPos).CrossProduct(m_vPoint[0] - normalPos);

	if ((sn01 * sn20) > 0 && (sn01 * sn12) > 0)
		return true;

	if (fabsf(sn01 * sn20 * sn12) < 1e-6)
		return true;

	return false;
}

bool MPainter2DRect::FillData(MIViewport* pViewport, MMesh<MPainterVertex>& mesh)
{
	if (nullptr == pViewport)
		return false;

	mesh.CreateVertices(GetVertexCount());
	mesh.CreateIndices(GetIndexCount(), 1);

	Vector4 color = m_rectColor.ToVector4();
	for (int i = 0; i < 4; ++i)
	{
		mesh.GetVertices()[i].pos = m_vPoint[i];
		mesh.GetVertices()[i].color = color;
	}

	static unsigned int const indices[] = {
		0, 1, 2,
		0, 2, 3
	};

	memcpy(mesh.GetIndices(), &indices, 2 * 3 * sizeof(unsigned int));

	mesh.SetNeedUpload();
	return true;
}

bool MPainter2DRect::TouchTest(const Vector2& pos, MIViewport* pViewport)
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

MPainter2DRect3D::MPainter2DRect3D(const Vector3& v3Center, const Vector3& v3Normal, const Vector3& v3Up, const MColor& rectColor, const float& fWidth, const float& fHeight) : MIPainterShape()
, m_v3Center(v3Center), m_v3Normal(v3Normal), m_v3Up(v3Up), m_rectColor(rectColor), m_fWidth(fWidth), m_fHeight(fHeight)
{
	m_v3Normal.Normalize();
	m_v3Up.Normalize();
}

bool MPainter2DRect3D::FillData(MIViewport* pViewport, MMesh<MPainterVertex>& mesh)
{
	if (nullptr == pViewport)
		return false;

	Vector2 v2Center;
	if (false == pViewport->ConvertWorldPointToNormalizedDevice(m_v3Center, v2Center))
		return false;

	float fHalfWidth = m_fWidth * 0.5;
	float fHalfHeight = m_fHeight * 0.5;
	
	Vector3 v3Right = m_v3Normal.CrossProduct(m_v3Up);
	v3Right.Normalize();

	Vector3 v3Pos[4] = {
		m_v3Center - v3Right * fHalfWidth - m_v3Up * fHalfHeight,
		m_v3Center - v3Right * fHalfWidth + m_v3Up * fHalfHeight,
		m_v3Center + v3Right * fHalfWidth - m_v3Up * fHalfHeight,
		m_v3Center + v3Right * fHalfWidth + m_v3Up * fHalfHeight,
	};
	
	Vector2 v2Pos[4];
	for (int i = 0; i < 4; ++i)
	{
		if (false == pViewport->ConvertWorldPointToNormalizedDevice(v3Pos[i], v2Pos[i]))
			return false;
	}

	mesh.CreateVertices(GetVertexCount());
	mesh.CreateIndices(GetIndexCount(), 1);

	Vector4 color = m_rectColor.ToVector4();
	for (int i = 0; i < 4; ++i)
	{
		mesh.GetVertices()[i].pos = v2Pos[i];
		mesh.GetVertices()[i].color = color;
	}

	static unsigned int const indices[] = {
		0, 1, 2,
		1, 3, 2
	};

	memcpy(mesh.GetIndices(), &indices, 2 * 3 * sizeof(unsigned int));

	mesh.SetNeedUpload();
	return true;
}

bool MPainter2DRect3D::TouchTest(const Vector2& pos, MIViewport* pViewport)
{
	return false;
}
