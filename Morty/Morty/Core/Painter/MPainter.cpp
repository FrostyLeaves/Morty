#include "MPainter.h"
#include "MMesh.h"
#include "MIViewport.h"
#include "MCamera.h"

MMesh<MPainterVertex>* MPainter2DLine::FillData(MIViewport* pViewport)
{
	if (nullptr == pViewport)
		return nullptr;

	float fHalfThickness = m_fThickness * 0.5f;

	Vector3 v2Begin2D = pViewport->ConvertWorldPositionTo2D(m_v3Begin);
	Vector3 v2End2D = pViewport->ConvertWorldPositionTo2D(m_v3End);

	float znear = pViewport->GetCamera()->GetZNear();

	if (v2Begin2D.z < znear && v2End2D.z < znear)
		return nullptr;

	if (v2Begin2D.z < znear && v2End2D.z >= znear)
	{
		Vector3 dir = (v2Begin2D - v2End2D);
		dir.Normalize();
		v2Begin2D = v2End2D + dir * (znear - v2End2D.z) / dir.z;
	}
	else if (v2Begin2D.z >= znear && v2End2D.z < znear)
	{
		Vector3 dir = (v2End2D - v2Begin2D);
		dir.Normalize();
		v2End2D = v2Begin2D + dir * (znear - v2Begin2D.z) / dir.z;
	}

	Vector2 v2Normal = v2End2D - v2Begin2D;
	if (fabs(v2Normal.x) < 0.0001)
		v2Normal = Vector2(1, 0);
	else
	{
		v2Normal = Vector2(-v2Normal.y / v2Normal.x, 1);
		v2Normal.Normalize();
	}


	MMesh<MPainterVertex>* pMesh = new MMesh<MPainterVertex>();
	pMesh->CreateVertices(GetVertexCount());
	pMesh->CreateIndices(GetIndexCount(), 1);

	pMesh->GetVertices()[0].pos.x = v2Begin2D.x + v2Normal.x * -fHalfThickness / pViewport->GetSize().x;
	pMesh->GetVertices()[0].pos.y = v2Begin2D.y + v2Normal.y * -fHalfThickness / pViewport->GetSize().y;
	pMesh->GetVertices()[0].color = m_lineColor.ToVector4();

	pMesh->GetVertices()[1].pos.x = v2Begin2D.x + v2Normal.x * fHalfThickness / pViewport->GetSize().x;
	pMesh->GetVertices()[1].pos.y = v2Begin2D.y + v2Normal.y * fHalfThickness / pViewport->GetSize().y;
	pMesh->GetVertices()[1].color = m_lineColor.ToVector4();

	pMesh->GetVertices()[2].pos.x = v2End2D.x + v2Normal.x * -fHalfThickness / pViewport->GetSize().x;
	pMesh->GetVertices()[2].pos.y = v2End2D.y + v2Normal.y * -fHalfThickness / pViewport->GetSize().y;
	pMesh->GetVertices()[2].color = m_lineColor.ToVector4();

	pMesh->GetVertices()[3].pos.x = v2End2D.x + v2Normal.x * fHalfThickness / pViewport->GetSize().x;
	pMesh->GetVertices()[3].pos.y = v2End2D.y + v2Normal.y * fHalfThickness / pViewport->GetSize().y;
	pMesh->GetVertices()[3].color = m_lineColor.ToVector4();

	static unsigned int const indices[] = {
		0, 1, 2,
		1, 3, 2
	};

	memcpy(pMesh->GetIndices(), &indices, 2 * 3 * sizeof(unsigned int));

	return pMesh;
}
