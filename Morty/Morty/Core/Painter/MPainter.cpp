#include "MPainter.h"
#include "MMesh.h"
#include "MIViewport.h"

struct Line
{
	MColor color;
	Vector3 begin;
	Vector3 end;
	float lineWidth;
};

struct Rect
{
	MColor color;
	Vector3 center;
	Vector3 normal;
	float width;
	float height;
};

struct Circle
{
	MColor color;
	Vector3 center;
	Vector3 normal;
	float radius;
	float angle;
};

MPainter::MPainter()
	: m_pViewport(nullptr)
{

}

MPainter::~MPainter()
{

}
#include "MCamera.h"
MIMesh* MPainter::GetMesh2DLine(const Vector3& v3Begin, const Vector3& v3End, const MColor& lineColor, const float& fThickness /*= 1.0f*/)
{
	if (nullptr == m_pViewport)
		return nullptr;

	float fHalfThickness = fThickness * 0.5f;


	Vector3 v2Begin2D = m_pViewport->ConvertWorldPositionTo2D(v3Begin);
	Vector3 v2End2D = m_pViewport->ConvertWorldPositionTo2D(v3End);

	float znear = m_pViewport->GetCamera()->GetZNear();

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
	pMesh->CreateVertices(4);
	pMesh->CreateIndices(2, 3);

	pMesh->GetVertices()[0].pos.x = v2Begin2D.x + v2Normal.x * -fHalfThickness / m_pViewport->GetSize().x;
	pMesh->GetVertices()[0].pos.y = v2Begin2D.y + v2Normal.y * -fHalfThickness / m_pViewport->GetSize().y;
	pMesh->GetVertices()[0].color = lineColor.ToVector4();

	pMesh->GetVertices()[1].pos.x = v2Begin2D.x + v2Normal.x * fHalfThickness / m_pViewport->GetSize().x;
	pMesh->GetVertices()[1].pos.y = v2Begin2D.y + v2Normal.y * fHalfThickness / m_pViewport->GetSize().y;
	pMesh->GetVertices()[1].color = lineColor.ToVector4();

	pMesh->GetVertices()[2].pos.x = v2End2D.x + v2Normal.x * -fHalfThickness / m_pViewport->GetSize().x;
	pMesh->GetVertices()[2].pos.y = v2End2D.y + v2Normal.y * -fHalfThickness / m_pViewport->GetSize().y;
	pMesh->GetVertices()[2].color = lineColor.ToVector4();

	pMesh->GetVertices()[3].pos.x = v2End2D.x + v2Normal.x * fHalfThickness / m_pViewport->GetSize().x;
	pMesh->GetVertices()[3].pos.y = v2End2D.y + v2Normal.y * fHalfThickness / m_pViewport->GetSize().y;
	pMesh->GetVertices()[3].color = lineColor.ToVector4();

	static unsigned int const indices[] = {
		0, 1, 2,
		1, 3, 2
	};

	memcpy(pMesh->GetIndices(), &indices, 2 * 3 * sizeof(unsigned int));


	return pMesh;
}

void MPainter::SetAttachedViewport(MIViewport* pViewport)
{
	m_pViewport = pViewport;
}

MPaintCollage::MPaintCollage()
	: m_lineColor(1.0f, 0.0f, 0.0f, 1.0f)
	, m_fillColor(1.0f, 0.0f, 0.0f, 1.0f)
	, m_fLineWidth(1)
{
}

MPaintCollage::~MPaintCollage()
{
	Clean();
}

void MPaintCollage::Clean()
{
}

void MPaintCollage::Draw2DLine(const Vector3& v3Begin, const Vector3& v3End)
{

}

MIMesh* MPaintCollage::GetRenderMesh(MIViewport* pViewport)
{
	return nullptr;
}