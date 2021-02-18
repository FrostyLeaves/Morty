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
	, m_pSelectedTexture(nullptr)
{

}

RenderGraphView::~RenderGraphView()
{

}

void RenderGraphView::SetRenderGraph(MRenderGraph* pRenderGraph)
{
	m_pRenderGraph = pRenderGraph;
}

MRenderGraphTexture* RenderGraphView::GetSelectedOutputTexture()
{
	return m_pSelectedTexture;
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

		float fInputTextMaxSize = 0;
		float fOutputTextMaxSize = 0;
		for (size_t i = 0; i < pNode->GetInputSize(); ++i)
		{
			if (MRenderGraphTexture* pInputTexture = pNode->GetInput(i)->GetLinkedTexture())
			{
				float fSize = ImGui::CalcTextSize(pInputTexture->GetTextureName().c_str()).x;
				if (fInputTextMaxSize < fSize)
					fInputTextMaxSize = fSize;
			}
		}

		for (size_t i = 0; i < pNode->GetOutputSize(); ++i)
		{
			if (MRenderGraphTexture* pInputTexture = pNode->GetOutput(i)->GetRenderTexture())
			{
				float fSize = ImGui::CalcTextSize(pInputTexture->GetTextureName().c_str()).x;
				if (fOutputTextMaxSize < fSize)
					fOutputTextMaxSize = fSize;
			}
		}

		for (size_t i = 0; i < pNode->GetInputSize(); ++i)
		{
			MString strIndex = pNode->GetInput(i)->GetStringID();
			ed::BeginPin(ImGui::GetID(strIndex.c_str()), ed::PinKind::Input);
			
			if (MRenderGraphTexture* pInputTexture = pNode->GetInput(i)->GetLinkedTexture())
			{
				ImGui::Text((MString("->") + pInputTexture->GetTextureName()).c_str());
			}
			ed::EndPin();
		}


		for (size_t i = 0; i < pNode->GetOutputSize(); ++i)
		{
			MString strIndex = pNode->GetOutput(i)->GetStringID();

			if (MRenderGraphTexture* pInputTexture = pNode->GetOutput(i)->GetRenderTexture())
			{
				float fWidth = ImGui::CalcTextSize(pInputTexture->GetTextureName().c_str()).x;

				ImGui::NewLine();
				ImGui::SameLine(fInputTextMaxSize + fOutputTextMaxSize + 50.0f - fWidth);

				MRenderGraphTexture* pOldSelectedTexture = m_pSelectedTexture;
				if (pInputTexture == pOldSelectedTexture)
				{
					const float fScale = 1.5f;
					ImVec4 color = ImGui::GetStyleColorVec4(ImGuiCol_Button);
					color.x *= fScale;
					color.y *= fScale;
					color.z *= fScale;

					ImGui::PushStyleColor(ImGuiCol_Button, color);
				}

				if (ImGui::Button(pInputTexture->GetTextureName().c_str()))
				{
					m_pSelectedTexture = pInputTexture;
				}

				if (pInputTexture == pOldSelectedTexture)
					ImGui::PopStyleColor(1);

				ImGui::SameLine();
				ed::BeginPin(ImGui::GetID(strIndex.c_str()), ed::PinKind::Output);
				ImGui::Text(MString("->").c_str());
				ed::EndPin();
			}
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
					MString strTargetIdx = pInput->GetStringID();
					
					ed::Link(++nPinIndex, ImGui::GetID(strIndex.c_str()), ImGui::GetID(strTargetIdx.c_str()));

				}
			}
		}
	}

	{
		int id = ImGui::GetID("Final Output");
		ed::BeginNode(id);
		ImGui::Text("Final Output");

		ed::BeginPin(ImGui::GetID("Final Input Pin"), ed::PinKind::Input);
		ImGui::Text("-> In");
		ed::EndPin();

		ed::EndNode();

		if (m_pRenderGraph)
		{
			if (MRenderGraphTexture* pOutputTexture = m_pRenderGraph->GetFinalOutputTexture())
			{
				if (MRenderGraphNodeOutput* pOutput = pOutputTexture->GetFinalNodeOutput())
				{
					MString strIndex = pOutput->GetStringID();
					ed::Link(++nPinIndex, ImGui::GetID(strIndex.c_str()), ImGui::GetID("Final Input Pin"));
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


