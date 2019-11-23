#include "MTransformCoord.h"
#include "M3DNode.h"
#include "MLogManager.h"

#include "MEngine.h"
#include "MIRenderer.h"
#include "MPainter.h"
#include "MMaterial.h"
#include "MResourceManager.h"
#include "MMaterialResource.h"
#include "MIViewport.h"

#include "MInputManager.h"

MTransformCoord3D::MTransformCoord3D()
	: MITransformCoord()
	, m_vDirection{Vector3(1, 0, 0), Vector3(0, 1, 0),Vector3(0, 0, 1)}
	, m_pTargetNode(nullptr)
	, m_eCoordHoverType(MECoordHoverType::None)
	, m_eCoordMoveType(0)
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

bool MTransformCoord3D::Input(MInputEvent* pEvent, MIViewport* pViewport)
{
	MMouseInputEvent* pMouseEvent = dynamic_cast<MMouseInputEvent*>(pEvent);
	if (nullptr == pMouseEvent)
		return false;

	if (nullptr == m_pTargetNode)
		return false;

	MPainter2DLine lines[3];
	bool vVaild[3];
	GetTranslationLines(lines, vVaild, pViewport);

	if (pMouseEvent->GetButton() == MMouseInputEvent::LeftButton)
	{
		if (pMouseEvent->GetType() == MMouseInputEvent::ButtonDown)
			m_eCoordMoveType = 1 << (int)m_eCoordHoverType - 1;
		else
			m_eCoordMoveType = 0;
	}

	if (m_eCoordMoveType != 0)	//正在拾取
	{
		Vector2 addi = pMouseEvent->GetMouseAddition();
		addi.y = -addi.y;

		Vector3 v3Origin = m_pTargetNode->GetParentWorldTransform() * m_pTargetNode->GetPosition();
		

		int unMoveAxisType = 1;
		for (int i = 0; i < 3; ++i)
		{
			if (m_eCoordMoveType & unMoveAxisType)
			{
				Vector2 dir = lines[i].GetDirection2D(pViewport);
				float value = addi * dir / dir.Length();
				Vector3 addiPosition(0, 0, 0);
				
				Vector2 pos1, pos2;
				pViewport->ConvertWorldLineToNormalizedDevice(v3Origin, v3Origin + m_vDirection[i], pos1, pos2);
				pos1 = (pos1 + Vector2(1.0, 1.0)) * 0.5;
				pos1.x *= pViewport->GetWidth();
				pos1.y *= pViewport->GetHeight();

				pos2 = (pos2 + Vector2(1.0, 1.0)) * 0.5;
				pos2.x *= pViewport->GetWidth();
				pos2.y *= pViewport->GetHeight();
				float fLength = (pos2 - pos1).Length();

				if (fLength < 1) fLength = 1.0f;
				addiPosition.m[i] = value / fLength;
				m_pTargetNode->SetPosition(m_pTargetNode->GetPosition() + addiPosition);
			}
			unMoveAxisType = unMoveAxisType << 1;
		}
	}
	else	//正在悬停
	{
		m_eCoordHoverType = MECoordHoverType::None;

		Vector2 pos = pMouseEvent->GetMosuePosition() - pViewport->GetLeftTop();
		pos.y = pViewport->GetHeight() - pos.y;
		for (int i = 0; i < 3; ++i)
		{
			if (vVaild[i] && lines[i].TouchTest(pos, pViewport))
			{
				m_eCoordHoverType = (MECoordHoverType)(i + 1);
				break;
			}
		}


	}

	return MECoordHoverType::None != m_eCoordHoverType;
}

void MTransformCoord3D::Render(MIRenderer* pRenderer, MIViewport* pViewport)
{
	if (nullptr == m_pTargetNode)
		return;

	static MMaterial* TestMaterial = nullptr;
	if (nullptr == TestMaterial)
	{
		MResource* pVSResource = m_pEngine->GetResourceManager()->Load("./Shader/draw.mvs");
		MResource* pPSResource = m_pEngine->GetResourceManager()->Load("./Shader/draw.mps");
		MMaterialResource* pMaterialRes = dynamic_cast<MMaterialResource*>(m_pEngine->GetResourceManager()->Create(MResourceManager::MEResourceType::Material));
		pMaterialRes->LoadVertexShader(pVSResource);
		pMaterialRes->LoadPixelShader(pPSResource);

		TestMaterial = pMaterialRes->GetMaterialTemplate();
	}

	pRenderer->SetUseMaterial(TestMaterial);
	pRenderer->UpdateMaterialParam();

	MPainter2DLine lines[3];
	bool vVaild[3];
	GetTranslationLines(lines, vVaild, pViewport);

	if (m_eCoordHoverType != MECoordHoverType::None)
	{
		lines[(int)m_eCoordHoverType - 1].m_lineColor = MColor(1, 1, 1, 1);
	}

	for (int i = 2; i >= 0; --i)
	{
		if (vVaild[i] && lines[i].FillData(pViewport, *static_cast<MMesh<MPainterVertex>*>(m_pCoordRenderCache)))
		{
			pRenderer->DrawMesh(m_pCoordRenderCache);
		}
	}
}

void MTransformCoord3D::GetTranslationLines(MPainter2DLine* lines, bool* vValid, MIViewport* pViewport)
{
	Vector3 v3Origin = m_pTargetNode->GetParentWorldTransform() * m_pTargetNode->GetPosition();
	Vector3 v3Right = v3Origin + Vector3(10, 0, 0);
	Vector3 v3Up = v3Origin + Vector3(0, 10, 0);
	Vector3 v3Forward = v3Origin + Vector3(0, 0, 10);

	float fMaxLength = 0.0f;
	Vector2 rit1, rit2, up1, up2, fwd1, fwd2;
	vValid[0] = pViewport->ConvertWorldLineToNormalizedDevice(v3Origin, v3Right, rit1, rit2);
	vValid[1] = pViewport->ConvertWorldLineToNormalizedDevice(v3Origin, v3Up, up1, up2);
	vValid[2] = pViewport->ConvertWorldLineToNormalizedDevice(v3Origin, v3Forward, fwd1, fwd2);

	if (vValid[0] && fMaxLength < (rit2 - rit1).Length())
		fMaxLength = (rit2 - rit1).Length();
	if (vValid[1] && fMaxLength < (up2 - up1).Length())
		fMaxLength = (up2 - up1).Length();
	if (vValid[2] && fMaxLength < (fwd2 - fwd1).Length())
		fMaxLength = (fwd2 - fwd1).Length();

	if (fMaxLength < 1e-6)
		fMaxLength = 1.0f;

	if (vValid[0])
	{
		rit2 = rit1 + (rit2 - rit1) * 0.5f / fMaxLength;
		lines[0] = MPainter2DLine(rit1, rit2, MColor(240.0f / 255.0f, 48.0f / 255.0f, 48.0f / 255.0f, 1), 4.0f);
	}
	if (vValid[1])
	{
		up2 = up1 + (up2 - up1) * 0.5f / fMaxLength;
		lines[1] = MPainter2DLine(up1, up2, MColor(60.0f / 255.0f, 179.0f / 255.0f, 113.0f / 255.0f, 1), 4.0f);
	}
	if (vValid[2])
	{
		fwd2 = fwd1 + (fwd2 - fwd1) * 0.5f / fMaxLength;
		lines[2] = MPainter2DLine(fwd1, fwd2, MColor(30.0f / 255.0f, 144.0f / 255.0f, 1, 1), 4.0f);
	}
}
