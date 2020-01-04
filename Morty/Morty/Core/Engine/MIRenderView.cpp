#include "MIRenderView.h"
#include "MViewport.h"

MIRenderView::MIRenderView()
	: m_pEngine(nullptr)
	, m_vViewport()
	, m_pRenderTarget(nullptr)
{

}

MIRenderView::~MIRenderView()
{

}

void MIRenderView::AppendViewport(MViewport* pViewport)
{
	for (MViewport* pvp : m_vViewport)
		if (pViewport == pvp)
			return;

	m_vViewport.push_back(pViewport);
}

void MIRenderView::RemoveViewport(MViewport* pViewport)
{
	std::vector<MViewport*>::iterator iter = std::find(m_vViewport.begin(), m_vViewport.end(), pViewport);
	if (iter != m_vViewport.end())
		m_vViewport.erase(iter);
}
