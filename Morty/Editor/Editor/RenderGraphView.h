#ifndef _RENDER_GRAPH_VIEW_H_
#define _RENDER_GRAPH_VIEW_H_

#include "IBaseView.h"

class MObject;
class MNode;
class MRenderGraph;
class MRenderGraphTexture;
class RenderGraphView : public IBaseView
{
public:
	RenderGraphView();
	virtual ~RenderGraphView();


	void SetRenderGraph(MRenderGraph* pRenderGraph);

	MRenderGraphTexture* GetSelectedOutputTexture();

public:
	virtual void Render() override;

	virtual void Initialize(MEngine* pEngine) override;
	virtual void Release() override;

	virtual void Input(MInputEvent* pEvent) override;


private:
	MEngine* m_pEngine;

	MRenderGraph* m_pRenderGraph;
	MRenderGraphTexture* m_pSelectedTexture;
};







#endif