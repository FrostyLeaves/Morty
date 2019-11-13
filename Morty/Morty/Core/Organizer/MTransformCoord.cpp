#include "MTransformCoord.h"
#include "M3DNode.h"

#include "MEngine.h"
#include "MIRenderer.h"
#include "MPainter.h"
#include "MMaterial.h"
#include "MResourceManager.h"
#include "MMaterialResource.h"

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
	m_pTargetNode = dynamic_cast<M3DNode*>(pNode);
}
// 
// bool Segment(const Vector2& a, const Vector2& b, const Vector2& m, const Vector2& n)
// {
// 	float sABM = (a.x - m.x) * (b.y - m.y) - (a.y - m.y) * (b.x - m.x);
// 	if (fabs(sABM) < 1e-6)
// 		return true;	
// 
// 	float sABN = (a.x - n.x) * (b.y - n.y) - (a.y - n.y) * (b.x - n.x);
// 	if (fabs(sABN) < 1e-6)
// 		return true;
// 
// 	if (sABM * sABN > 0)
// 		return false;
// 
// 	float sAMN = (m.x - a.x) * (n.y - a.y) - (m.y - a.y) * (n.x - a.x);
// 
// 	float sBMN = sAMN + sABM - sABN;
// 	if (sAMN * sBMN >= 0)
// 		return false;
// 
// 	return true;
// }

#include "MIViewport.h"
#include "MLogManager.h"
bool MTransformCoord3D::Input(MInputEvent* pEvent, MIViewport* pViewport)
{
	MMouseInputEvent* pMouseEvent = dynamic_cast<MMouseInputEvent*>(pEvent);
	if (nullptr == pMouseEvent)
		return false;

	if (nullptr == m_pTargetNode)
		return false;

	m_eCoordHoverType = MECoordHoverType::None;

	MPainter2DLine lines[] = {
	MPainter2DLine(m_pTargetNode->GetParentWorldTransform() * m_pTargetNode->GetPosition(), m_pTargetNode->GetParentWorldTransform() * (m_pTargetNode->GetPosition() + Vector3(10, 0, 0)), MColor(1, 0, 0, 1), 6.0f),
	MPainter2DLine(m_pTargetNode->GetParentWorldTransform() * m_pTargetNode->GetPosition(), m_pTargetNode->GetParentWorldTransform() * (m_pTargetNode->GetPosition() + Vector3(0, 10, 0)), MColor(0, 1, 0, 1), 6.0f),
	MPainter2DLine(m_pTargetNode->GetParentWorldTransform() * m_pTargetNode->GetPosition(), m_pTargetNode->GetParentWorldTransform() * (m_pTargetNode->GetPosition() + Vector3(0, 0, 10)), MColor(30.0f / 255.0f, 144.0f / 255.0f, 1, 1), 6.0f)
	};
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
				if (fLength > 1e-6)
				{
					addiPosition.m[i] = value / fLength * 10;
					m_pTargetNode->SetPosition(m_pTargetNode->GetPosition() + addiPosition);
				}
			}
			unMoveType *= 2;
		}		
	}

	if (pMouseEvent->GetButton() == MMouseInputEvent::LeftButton)
	{
		if (pMouseEvent->GetType() == MMouseInputEvent::ButtonDown)
			m_eCoordMoveType = pow(2, (double)m_eCoordHoverType - 1);
		else
			m_eCoordMoveType = 0;
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

	MPainter2DLine lines[] = {
	MPainter2DLine(m_pTargetNode->GetParentWorldTransform() * m_pTargetNode->GetPosition(), m_pTargetNode->GetParentWorldTransform() * (m_pTargetNode->GetPosition() + Vector3(10, 0, 0)), MColor(1, 0, 0, 1), 6.0f),
	MPainter2DLine(m_pTargetNode->GetParentWorldTransform() * m_pTargetNode->GetPosition(), m_pTargetNode->GetParentWorldTransform() * (m_pTargetNode->GetPosition() + Vector3(0, 10, 0)), MColor(0, 1, 0, 1), 6.0f),
	MPainter2DLine(m_pTargetNode->GetParentWorldTransform() * m_pTargetNode->GetPosition(), m_pTargetNode->GetParentWorldTransform() * (m_pTargetNode->GetPosition() + Vector3(0, 0, 10)), MColor(30.0f / 255.0f, 144.0f / 255.0f, 1, 1), 6.0f)
	};

	if (m_eCoordHoverType != MECoordHoverType::None)
	{
		lines[(int)m_eCoordHoverType - 1].m_lineColor = MColor(1, 1, 1, 1);
	}

	for (int i = 0; i < 3; ++i)
	{
		if (lines[i].FillData(pViewport, *static_cast<MMesh<MPainterVertex>*>(m_pCoordRenderCache)))
		{
			pRenderer->DrawMesh(m_pCoordRenderCache);
		}
	}
}
