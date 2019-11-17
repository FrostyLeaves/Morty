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
	GetTranslationLines(lines);

	if (pMouseEvent->GetButton() == MMouseInputEvent::LeftButton)
	{
		if (pMouseEvent->GetType() == MMouseInputEvent::ButtonDown)
			m_eCoordMoveType = 1 << (int)m_eCoordHoverType - 1;
		else
			m_eCoordMoveType = 0;
	}
	
	if (m_eCoordMoveType != 0)
	{
		Vector2 addi = pMouseEvent->GetMouseAddition();
		addi.y = -addi.y;

		int unMoveType = 1;
		for (int i = 0; i < 3; ++i)
		{
			if (m_eCoordMoveType & unMoveType)
			{
				Vector2 dir = lines[i].GetDirection2D(pViewport);
				float value = addi * dir / dir.Length();
				Vector3 addiPosition(0, 0, 0);
				float fLength = lines[i].GetLength2D(pViewport);
				
				if (fLength < 1) fLength = 1.0f;
				addiPosition.m[i] = value / fLength * 10;
				m_pTargetNode->SetPosition(m_pTargetNode->GetPosition() + addiPosition);
				
			}
			unMoveType *= 2;
		}		
	}
	else
	{
		m_eCoordHoverType = MECoordHoverType::None;

		Vector2 pos = pMouseEvent->GetMosuePosition() - pViewport->GetLeftTop();
		pos.y = pViewport->GetHeight() - pos.y;
		for (int i = 0; i < 3; ++i)
		{
			if (lines[i].TouchTest(pos, pViewport))
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
		TestMaterial = m_pEngine->GetObjectManager()->CreateObject<MMaterial>();
		MResource* pVSResource = m_pEngine->GetResourceManager()->Load("./Shader/draw.mvs");
		MResource* pPSResource = m_pEngine->GetResourceManager()->Load("./Shader/draw.mps");
		MMaterialResource* pMaterialRes = dynamic_cast<MMaterialResource*>(m_pEngine->GetResourceManager()->Create(MResourceManager::MEResourceType::Material));
		pMaterialRes->LoadVertexShader(pVSResource);
		pMaterialRes->LoadPixelShader(pPSResource);

		TestMaterial->Load(pMaterialRes);
	}

	pRenderer->SetUseMaterial(TestMaterial);
	pRenderer->UpdateMaterialParam();

	MPainter2DLine lines[3];
	GetTranslationLines(lines);

	if (m_eCoordHoverType != MECoordHoverType::None)
	{
		lines[(int)m_eCoordHoverType - 1].m_lineColor = MColor(1, 1, 1, 1);
	}

	for (int i = 2; i >= 0; --i)
	{
		if (lines[i].FillData(pViewport, *static_cast<MMesh<MPainterVertex>*>(m_pCoordRenderCache)))
		{
			pRenderer->DrawMesh(m_pCoordRenderCache);
		}
	}
}

void MTransformCoord3D::GetTranslationLines(MPainter2DLine* lines)
{

	lines[0] = MPainter2DLine(m_pTargetNode->GetParentWorldTransform() * m_pTargetNode->GetPosition(), m_pTargetNode->GetParentWorldTransform() * (m_pTargetNode->GetPosition() + Vector3(10, 0, 0)), MColor(240.0f / 255.0f, 48.0f / 255.0f, 48.0f / 255.0f, 1), 4.0f);
	lines[1] = MPainter2DLine(m_pTargetNode->GetParentWorldTransform() * m_pTargetNode->GetPosition(), m_pTargetNode->GetParentWorldTransform() * (m_pTargetNode->GetPosition() + Vector3(0, 10, 0)), MColor(60.0f / 255.0f, 179.0f / 255.0f, 113.0f / 255.0f, 1), 4.0f);
	lines[2] = MPainter2DLine(m_pTargetNode->GetParentWorldTransform() * m_pTargetNode->GetPosition(), m_pTargetNode->GetParentWorldTransform() * (m_pTargetNode->GetPosition() + Vector3(0, 0, 10)), MColor(30.0f / 255.0f, 144.0f / 255.0f, 1, 1), 4.0f);
}
