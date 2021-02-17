#include "RenderGraphView.h"

#include "MEngine.h"
#include "MObject.h"

#include "imgui.h"
#include "imgui_node_editor.h"
#include "MNode.h"

#include "MRenderGraph.h"

namespace ed = ax::NodeEditor;


static ed::EditorContext* g_Context = nullptr;

RenderGraphView::RenderGraphView()
	: IBaseView()
	, m_pEngine(nullptr)
	, m_pRenderGraph(nullptr)
{

}

RenderGraphView::~RenderGraphView()
{

}

void RenderGraphView::SetRenderGraph(MRenderGraph* pRenderGraph)
{
	m_pRenderGraph = pRenderGraph;
}

void RenderGraphView::Render()
{
	if (!m_pRenderGraph)
		return;

	int nPinIndex = 0;

	ed::SetCurrentEditor(g_Context);

	ed::Begin("My Editor");

	auto& vNodes = m_pRenderGraph->GetAllNodes();

	for (MRenderGraphNode* pNode : vNodes)
	{
		int id = ImGui::GetID(pNode->GetNodeName().c_str());
		ed::BeginNode(id);

		ImGui::Text(pNode->GetNodeName().c_str());

		for (size_t i = 0; i < pNode->GetInputSize(); ++i)
		{
			MString strIndex = pNode->GetNodeName() + "_Input_" + MStringHelper::ToString(i);
			ed::BeginPin(ImGui::GetID(strIndex.c_str()), ed::PinKind::Input);
			ImGui::Text("-> In");
			ed::EndPin();
		}
		for (size_t i = 0; i < pNode->GetOutputSize(); ++i)
		{
			MString strIndex = pNode->GetNodeName() + "_Output_" + MStringHelper::ToString(i);
			ed::BeginPin(ImGui::GetID(strIndex.c_str()), ed::PinKind::Output);
			ImGui::Text("Out ->");
			ed::EndPin();
		}


		ed::EndNode();
	}

	for (MRenderGraphNode* pNode : vNodes)
	{
		for (size_t i = 0; i < pNode->GetOutputSize(); ++i)
		{
			MString strIndex = pNode->GetNodeName() + "_Output_" + MStringHelper::ToString(i);
			
			if (MRenderGraphNodeOutput* pOutput = pNode->GetOutput(i))
			{
				for (MRenderGraphNodeInput* pInput : pOutput->GetLinkedInputs())
				{
					MString strTargetIdx = pInput->GetGraphNode()->GetNodeName() + "_Input_" + MStringHelper::ToString(pInput->GetIndex());
					
					ed::Link(++nPinIndex, ImGui::GetID(strIndex.c_str()), ImGui::GetID(strTargetIdx.c_str()));

				}
			}
		}
	}
		 
	ed::End();
}

void RenderGraphView::Initialize(MEngine* pEngine)
{
	m_pEngine = pEngine;

	if (nullptr == g_Context)
	{
		g_Context = ed::CreateEditor();
	}
}

void RenderGraphView::Release()
{
	if (g_Context)
	{
		ed::DestroyEditor(g_Context);
	}
}

void RenderGraphView::Input(MInputEvent* pEvent)
{

}


