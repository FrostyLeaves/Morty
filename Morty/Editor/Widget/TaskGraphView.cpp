#include "TaskGraphView.h"

#include "Engine/MEngine.h"
#include "Object/MObject.h"
#include "TaskGraph/MTaskNode.h"
#include "TaskGraph/MTaskGraph.h"
#include "TaskGraph/MTaskNodeInput.h"
#include "TaskGraph/MTaskNodeOutput.h"

#include "imgui.h"
#include "imgui_node_editor.h"

namespace ed = ax::NodeEditor;


static ed::EditorContext* g_Context = nullptr;

TaskGraphView::TaskGraphView()
	: IBaseView()
	, m_pEngine(nullptr)
	, m_pRenderGraph(nullptr)
{

}

TaskGraphView::~TaskGraphView()
{

}

void TaskGraphView::SetRenderGraph(MTaskGraph* pRenderGraph)
{
	if (m_pRenderGraph = pRenderGraph)
	{

	}
}

void TaskGraphView::Render()
{
	if (!m_pRenderGraph)
		return;

	int nPinIndex = 0;

	ed::SetCurrentEditor(g_Context);

	ed::Begin("My Editor");

	auto& vNodes = m_pRenderGraph->GetAllNodes();

	for (MTaskNode* pNode : vNodes)
	{
		int id = ImGui::GetID(pNode->GetNodeName().c_str());
		ed::BeginNode(id);

		ImGui::Text(pNode->GetNodeName().c_str());

		float fInputTextMaxSize = 0;
		float fOutputTextMaxSize = 0;
		for (size_t i = 0; i < pNode->GetInputSize(); ++i)
		{
			if (MTaskNodeInput* pInput = pNode->GetInput(i))
			{
				float fSize = ImGui::CalcTextSize(pInput->GetName().c_str()).x;
				if (fInputTextMaxSize < fSize)
					fInputTextMaxSize = fSize;
			}
		}

		for (size_t i = 0; i < pNode->GetOutputSize(); ++i)
		{
			if (MTaskNodeOutput* pOutput = pNode->GetOutput(i))
			{
				float fSize = ImGui::CalcTextSize(pOutput->GetName().c_str()).x;
				if (fOutputTextMaxSize < fSize)
					fOutputTextMaxSize = fSize;
			}
		}

		for (size_t i = 0; i < pNode->GetInputSize(); ++i)
		{
			MString strIndex = pNode->GetInput(i)->GetStringID();
			ed::BeginPin(ImGui::GetID(strIndex.c_str()), ed::PinKind::Input);
			
			if (MTaskNodeInput* pInput = pNode->GetInput(i))
			{
				ImGui::Text((MString("->") + pInput->GetName()).c_str());
			}
			ed::EndPin();
		}


		for (size_t i = 0; i < pNode->GetOutputSize(); ++i)
		{
			MString strIndex = pNode->GetOutput(i)->GetStringID();

			if (MTaskNodeOutput* pOutput = pNode->GetOutput(i))
			{
				float fWidth = ImGui::CalcTextSize(pOutput->GetName().c_str()).x;

				ImGui::NewLine();
				ImGui::SameLine(fInputTextMaxSize + fOutputTextMaxSize + 50.0f - fWidth);

				MTaskNodeOutput* pOldSelectedOutput = m_pSelectedOutput;
				if (pOutput == pOldSelectedOutput)
				{
					const float fScale = 1.5f;
					ImVec4 color = ImGui::GetStyleColorVec4(ImGuiCol_Button);
					color.x *= fScale;
					color.y *= fScale;
					color.z *= fScale;

					ImGui::PushStyleColor(ImGuiCol_Button, color);
				}

				if (ImGui::Button((pOutput->GetName() + "##" + pNode->GetOutput(i)->GetStringID()).c_str()))
				{
					m_pSelectedOutput = pOutput;
				}

				if (pOutput == pOldSelectedOutput)
					ImGui::PopStyleColor(1);

				ImGui::SameLine();
				ed::BeginPin(ImGui::GetID(strIndex.c_str()), ed::PinKind::Output);
				ImGui::Text(MString("->").c_str());
				ed::EndPin();
			}
		}

		ed::EndNode();
	}

	for (MTaskNode* pNode : vNodes)
	{
		for (size_t i = 0; i < pNode->GetOutputSize(); ++i)
		{
			MString strIndex = pNode->GetNodeName() + "_Output_" + MStringHelper::ToString(i);
			
			if (MTaskNodeOutput* pOutput = pNode->GetOutput(i))
			{
				for (MTaskNodeInput* pInput : pOutput->GetLinkedInputs())
				{
					MString strTargetIdx = pInput->GetStringID();
					
					ed::Link(++nPinIndex, ImGui::GetID(strIndex.c_str()), ImGui::GetID(strTargetIdx.c_str()));

				}
			}
		}
	}

	ed::End();
}

void TaskGraphView::Initialize(MEngine* pEngine)
{
	m_pEngine = pEngine;

	if (nullptr == g_Context)
	{
		g_Context = ed::CreateEditor();
	}
}

void TaskGraphView::Release()
{
	if (g_Context)
	{
		ed::DestroyEditor(g_Context);
	}
}

void TaskGraphView::Input(MInputEvent* pEvent)
{

}


