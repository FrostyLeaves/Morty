#include "MIRenderView.h"
#include "MIViewport.h"

MIRenderView::MIRenderView()
	: m_pEngine(nullptr)
	, m_vViewport()
{

}

MIRenderView::~MIRenderView()
{

}

void MIRenderView::AppendViewport(MIViewport* pViewport)
{
	for (MIViewport* pvp : m_vViewport)
		if (pViewport == pvp)
			return;

	m_vViewport.push_back(pViewport);
}

void MIRenderView::RemoveViewport(MIViewport* pViewport)
{
	std::vector<MIViewport*>::iterator iter = std::find(m_vViewport.begin(), m_vViewport.end(), pViewport);
	if (iter != m_vViewport.end())
		m_vViewport.erase(iter);
}
