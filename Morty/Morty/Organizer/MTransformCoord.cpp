#include "MTransformCoord.h"

#include <algorithm>

#include "M3DNode.h"
#include "MLogManager.h"

#include "MEngine.h"
#include "MIRenderer.h"
#include "MPainter.h"
#include "MMaterial.h"
#include "MResourceManager.h"
#include "Material/MMaterialResource.h"
#include "MViewport.h"
#include "MCamera.h"

#include "MInputManager.h"

#define MOVE(pos, from, to, percent) ((pos) + ((to) - (from)) * percent)
#define TEMP(a, b, temp) {temp = a; a = b; b = temp;}

const Vector3 MTransformCoord3D::m_vDirection[3] = { Vector3(1, 0, 0), Vector3(0, 1, 0), Vector3(0, 0, 1) };
const MColor MTransformCoord3D::m_vColor[3] = { MColor(240.0f / 255.0f, 48.0f / 255.0f, 48.0f / 255.0f, 0.5f), MColor(60.0f / 255.0f, 179.0f / 255.0f, 113.0f / 255.0f, 0.5f), MColor(30.0f / 255.0f, 144.0f / 255.0f, 1, 0.5f) };


MTransformCoord3D::MTransformCoord3D()
	: MITransformCoord()
	, m_pTargetNode(nullptr)
	, m_eCoordHoverType(MECoordHoverType::None)
	, m_eCoordMoveType(MECoordHoverType::None)
	, m_pCoordRenderCache(new MMesh<MPainterVertex>(true))
{

}

MTransformCoord3D::~MTransformCoord3D()
{
	if (m_pCoordRenderCache)
	{
		m_pCoordRenderCache->DestroyBuffer(m_pEngine->GetDevice());
		delete m_pCoordRenderCache;
		m_pCoordRenderCache = nullptr;
	}
}

void MTransformCoord3D::SetTarget3DNode(MNode* pNode)
{
	if (m_pTargetNode == dynamic_cast<M3DNode*>(pNode))
		return;

	m_pTargetNode = dynamic_cast<M3DNode*>(pNode);
}

bool MTransformCoord3D::Input(MInputEvent* pEvent, MViewport* pViewport)
{
	MMouseInputEvent* pMouseEvent = dynamic_cast<MMouseInputEvent*>(pEvent);
	if (nullptr == pMouseEvent)
		return false;

	if (nullptr == m_pTargetNode)
		return false;

	bool vVaild[3];
	int vOrder[3];
	MPainter2DLine lines[3];
	MPainter2DRect rects[3];
	GetTranslationShapes(lines, rects, vVaild, vOrder, pViewport);


	if (pMouseEvent->GetButton() == MMouseInputEvent::LeftButton)
	{
		if (pMouseEvent->GetType() == MMouseInputEvent::ButtonDown)
		{
			m_eCoordMoveType = m_eCoordHoverType;

			
			m_v3TransformOrigin = m_pTargetNode->GetParentWorldTransform() * m_pTargetNode->GetPosition();
			const Vector3& v3Origin = m_v3TransformOrigin;

			switch (m_eCoordMoveType)
			{
			case MECoordHoverType::X:
			case MECoordHoverType::Y:
			case MECoordHoverType::Z:
			{
				unsigned int i = GetAxisIndex(m_eCoordMoveType);

				Vector2 pos1, pos2;
				pViewport->ConvertWorldLineToNormalizedDevice(v3Origin, v3Origin + m_vDirection[i], pos1, pos2);
				pos1 = (pos1 + Vector2(1.0, 1.0)) * 0.5;
				pos1.x *= pViewport->GetWidth();
				pos1.y *= pViewport->GetHeight();

				pos2 = (pos2 + Vector2(1.0, 1.0)) * 0.5;
				pos2.x *= pViewport->GetWidth();
				pos2.y *= pViewport->GetHeight();

				m_v3NormalizedDirection = pos2 - pos1;
				break;
			}

			case MECoordHoverType::XY:
			case MECoordHoverType::XZ:
			case MECoordHoverType::YZ:
			{


				
				break;
			}

			default:
				break;
			}
		}
		else
			m_eCoordMoveType = MECoordHoverType::None;
	}

	if (m_eCoordMoveType != MECoordHoverType::None)	//正在拾取
	{
		Vector2 addi = pMouseEvent->GetMouseAddition();
		addi.y = -addi.y;

		Vector3 v3Origin = m_pTargetNode->GetParentWorldTransform() * m_pTargetNode->GetPosition();
		
		switch (m_eCoordMoveType)
		{
		case MECoordHoverType::X:
		case MECoordHoverType::Y:
		case MECoordHoverType::Z:
		{
			unsigned int i = GetAxisIndex(m_eCoordMoveType);
			
			Vector3 addiPosition(0, 0, 0);

			float fLength = m_v3NormalizedDirection.Length();
			if (fLength < 1e-6) fLength = 1.0f;

			addiPosition.m[i] = Vector3(addi, 0.0f) * m_v3NormalizedDirection / (fLength * fLength);
			m_pTargetNode->SetPosition(m_pTargetNode->GetPosition() + addiPosition);

			break;
		}

		case MECoordHoverType::XY:
		case MECoordHoverType::XZ:
		case MECoordHoverType::YZ:
		{
			Vector3 v3CameraPos = pViewport->GetCamera()->GetWorldPosition();

			unsigned int i = GetAxisIndex((MECoordHoverType)((int)MECoordHoverType::XYZ ^ (int)m_eCoordMoveType));

			GetIntersection(v3CameraPos,)


			break;
		}

		default:
			break;
		}

	}
	else	//正在悬停
	{
		m_eCoordHoverType = MECoordHoverType::None;

		Vector2 pos = pMouseEvent->GetMosuePosition() - pViewport->GetLeftTop();
		pos.y = pViewport->GetHeight() - pos.y;
		for (int i : vOrder)
		{
			if (vVaild[i] && lines[i].TouchTest(pos, pViewport))
			{
				m_eCoordHoverType = (MECoordHoverType)(1 << (i));
				break;
			}
		}

		if (MECoordHoverType::None == m_eCoordHoverType)
		{
			for (int oi =2; oi >= 0; --oi)
			{
				int i = vOrder[oi];
				if (vVaild[(i + 1) % 3] && vVaild[(i + 2) % 3] && rects[i].TouchTest(pos, pViewport))
				{
					m_eCoordHoverType = (MECoordHoverType)(1 << ((i + 1) % 3) | 1 << ((i + 2) % 3));
					break;
				}
			}
		}

	}

	return MECoordHoverType::None != m_eCoordHoverType;
}

void MTransformCoord3D::Render(MIRenderer* pRenderer, MViewport* pViewport)
{
	if (nullptr == m_pTargetNode)
		return;

	MMaterialResource* pMaterialRes = m_pEngine->GetResourceManager()->LoadVirtualResource<MMaterialResource>(DEFAULT_MATERIAL_DRAW2D);
	MMaterial* pMaterial = pMaterialRes->GetMaterialTemplate();

	if (!pRenderer->SetUseMaterial(pMaterial))
		return;

	pRenderer->UpdateMaterialParam();

	bool vVaild[3];
	int vOrder[3];
	MPainter2DLine lines[3];
	MPainter2DRect rects[3];
	GetTranslationShapes(lines, rects, vVaild, vOrder, pViewport);

	if (false == (vVaild[0] && vVaild[1] && vVaild[2]))
		return;

	if (m_eCoordHoverType != MECoordHoverType::None)
	{
		for (int i : vOrder)
		{
			if (m_eCoordHoverType == (MECoordHoverType)(1 << i))
				lines[i].m_lineColor = MColor(1, 1, 1, 1);
			else if ((int)m_eCoordHoverType == (1 << ((i + 1) % 3) | 1 << ((i + 2) % 3)))
				rects[i].m_rectColor = MColor(1, 1, 1, 1);
		}
	}

	for (int oi = 2; oi >= 0; --oi)
	{
		int i = vOrder[oi];
		if (vVaild[i] && lines[i].FillData(pViewport, *static_cast<MMesh<MPainterVertex>*>(m_pCoordRenderCache)))
		{
		}
	}

	for (int oi = 0; oi < 3; ++oi)
	{
		int i = vOrder[oi];
		if (vVaild[(i + 1) % 3] && vVaild[(i + 2) % 3] && rects[i].FillData(pViewport, *static_cast<MMesh<MPainterVertex>*>(m_pCoordRenderCache)))
		{
		}
	}

	if (m_pCoordRenderCache->GetIndicesLength() > 0)
	{
		pRenderer->DrawMesh(m_pCoordRenderCache);
		m_pCoordRenderCache->Clean();
	}
}

void MTransformCoord3D::GetTranslationShapes(MPainter2DLine* lines, class MPainter2DRect* rects, bool* vValid, int* vOrder, MViewport* pViewport)
{
	Vector3 v3Origin = m_pTargetNode->GetParentWorldTransform() * m_pTargetNode->GetPosition();
	Vector3 v3EndPoint[3] = {
		v3Origin + m_vDirection[0] * 10,
		v3Origin + m_vDirection[1] * 10,
		v3Origin + m_vDirection[2] * 10
	};

	float fMaxLength = 0.0001f;
	Vector2 rit1, rit2, up1, up2, fwd1, fwd2;
	vValid[0] = pViewport->ConvertWorldLineToNormalizedDevice(v3Origin, v3EndPoint[0], rit1, rit2);
	vValid[1] = pViewport->ConvertWorldLineToNormalizedDevice(v3Origin, v3EndPoint[1], up1, up2);
	vValid[2] = pViewport->ConvertWorldLineToNormalizedDevice(v3Origin, v3EndPoint[2], fwd1, fwd2);

	if (vValid[0] && fMaxLength < (rit2 - rit1).Length())
		fMaxLength = (rit2 - rit1).Length();
	if (vValid[1] && fMaxLength < (up2 - up1).Length())
		fMaxLength = (up2 - up1).Length();
	if (vValid[2] && fMaxLength < (fwd2 - fwd1).Length())
		fMaxLength = (fwd2 - fwd1).Length();

	if (vValid[0])
	{
		rit2 = rit1 + (rit2 - rit1) * (200.0f / pViewport->GetHeight()) / fMaxLength;
		lines[0] = MPainter2DLine(rit1, rit2, m_vColor[0], 6.0f);
	}
	if (vValid[1])
	{
		up2 = up1 + (up2 - up1) * (200.0f / pViewport->GetHeight()) / fMaxLength;
		lines[1] = MPainter2DLine(up1, up2, m_vColor[1], 6.0f);
	}
	if (vValid[2])
	{
		fwd2 = fwd1 + (fwd2 - fwd1) * (200.0f / pViewport->GetHeight()) / fMaxLength;
		lines[2] = MPainter2DLine(fwd1, fwd2, m_vColor[2], 6.0f);
	}

	if (vValid[1] && vValid[2])
	{
		Vector2 sp = MOVE(MOVE(up1, up1, up2, 0.25), fwd1, fwd2, 0.25);
		rects[0] = MPainter2DRect(sp, MOVE(sp, up1, up2, 0.25), MOVE(MOVE(sp, up1, up2, 0.25), fwd1, fwd2, 0.25), MOVE(sp, fwd1, fwd2, 0.25), m_vColor[0]);
	}
	if (vValid[0] && vValid[2])
	{
		Vector2 sp = MOVE(MOVE(rit1, rit1, rit2, 0.25), fwd1, fwd2, 0.25);
		rects[1] = MPainter2DRect(sp, MOVE(sp, rit1, rit2, 0.25), MOVE(MOVE(sp, rit1, rit2, 0.25), fwd1, fwd2, 0.25), MOVE(sp, fwd1, fwd2, 0.25), m_vColor[1]);
	}
	if (vValid[0] && vValid[1])
	{
		Vector2 sp = MOVE(MOVE(up1, up1, up2, 0.25), rit1, rit2, 0.25);
		rects[2] = MPainter2DRect(sp, MOVE(sp, up1, up2, 0.25), MOVE(MOVE(sp, up1, up2, 0.25), rit1, rit2, 0.25), MOVE(sp, rit1, rit2, 0.25), m_vColor[2]);
	}

	vOrder[0] = 0;
	vOrder[1] = 1;
	vOrder[2] = 2;

	pViewport->ConvertWorldPointToViewport(v3EndPoint[0], v3EndPoint[0]);
	pViewport->ConvertWorldPointToViewport(v3EndPoint[1], v3EndPoint[1]);
	pViewport->ConvertWorldPointToViewport(v3EndPoint[2], v3EndPoint[2]);

	std::sort(vOrder, vOrder + 3, [&v3EndPoint](int a, int b)
		{
			return (v3EndPoint[a].z < v3EndPoint[b].z);
		});
}

unsigned int MTransformCoord3D::GetAxisIndex(const MECoordHoverType& eType)
{
	switch (eType)
	{
	case MECoordHoverType::X:
		return 0;
	case MECoordHoverType::Y:
		return 1;
	case MECoordHoverType::Z:
		return 2;
	default:
		return 3;
	}
}

bool MTransformCoord3D::GetIntersection(const Vector3& v3Origin, const Vector3& v3Direction, const Vector3& v3PlaneOrigin, const Vector3& v3PlaneNormal)
{
	Vector3 v3Result;

	float fLength = (v3PlaneNormal * v3PlaneOrigin - v3PlaneNormal * v3Origin) / (v3PlaneNormal * v3Direction);

	if (fLength < 0.0f)
		return false;

	v3Result = v3Origin + v3Direction * fLength;

	return true;
}
