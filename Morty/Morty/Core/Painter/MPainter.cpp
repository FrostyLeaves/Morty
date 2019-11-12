#include "MPainter.h"
#include "MMesh.h"
#include "MIViewport.h"
#include "MCamera.h"

bool MPainter2DLine::FillData(MIViewport* pViewport, MMesh<MPainterVertex>& mesh)
{
	if (nullptr == pViewport)
		return false;

	float fHalfThickness = m_fThickness * 0.5f;

	Vector3 v2Begin2D, v2End2D;
	if (false == pViewport->ConvertWorldLineToNormalizedDevice(m_v3Begin, m_v3End, v2Begin2D, v2End2D))
		return false;

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

	mesh.GetVertices()[0].pos.x = v2Begin2D.x + v2Normal.x * -fHalfThickness / pViewport->GetSize().x;
	mesh.GetVertices()[0].pos.y = v2Begin2D.y + v2Normal.y * -fHalfThickness / pViewport->GetSize().y;
	mesh.GetVertices()[0].color = m_lineColor.ToVector4();

	mesh.GetVertices()[1].pos.x = v2Begin2D.x + v2Normal.x * fHalfThickness / pViewport->GetSize().x;
	mesh.GetVertices()[1].pos.y = v2Begin2D.y + v2Normal.y * fHalfThickness / pViewport->GetSize().y;
	mesh.GetVertices()[1].color = m_lineColor.ToVector4();

	mesh.GetVertices()[2].pos.x = v2End2D.x + v2Normal.x * -fHalfThickness / pViewport->GetSize().x;
	mesh.GetVertices()[2].pos.y = v2End2D.y + v2Normal.y * -fHalfThickness / pViewport->GetSize().y;
	mesh.GetVertices()[2].color = m_lineColor.ToVector4();

	mesh.GetVertices()[3].pos.x = v2End2D.x + v2Normal.x * fHalfThickness / pViewport->GetSize().x;
	mesh.GetVertices()[3].pos.y = v2End2D.y + v2Normal.y * fHalfThickness / pViewport->GetSize().y;
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

	Vector3 v2Begin2D, v2End2D;
	if (false == pViewport->ConvertWorldLineToNormalizedDevice(m_v3Begin, m_v3End, v2Begin2D, v2End2D))
		return nullptr;

	v2Begin2D = (v2Begin2D + Vector2(1.0, 1.0)) * 0.5;
	v2Begin2D.x *= pViewport->GetWidth();
	v2Begin2D.y *= pViewport->GetHeight();

	v2End2D = (v2End2D + Vector2(1.0, 1.0)) * 0.5;
	v2End2D.x *= pViewport->GetWidth();
	v2End2D.y *= pViewport->GetHeight();


	float fValue = ((v2End2D.x - v2Begin2D.x) * (pos.x - v2Begin2D.x) + (v2End2D.y - v2Begin2D.y) * (pos.y - v2Begin2D.y)) / ((v2End2D.x - v2Begin2D.x) * (v2End2D.x - v2Begin2D.x) + (v2End2D.y - v2Begin2D.y) * (v2End2D.y - v2Begin2D.y));
	Vector2 v2Projection = v2Begin2D + (v2End2D - v2Begin2D) * fValue;

	if ((pos - v2Projection).Length() <= fHalfThickness)
		return true;

	return false;
}
