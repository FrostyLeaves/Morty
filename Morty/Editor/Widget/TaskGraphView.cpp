#include "TaskGraphView.h"

#include "GraphEditor.h"
#include "Engine/MEngine.h"

#include "Scene/MEntity.h"
#include "TaskGraph/MTaskGraph.h"

#include "Render/ImGui/imnodes.h"

using namespace morty;

TaskGraphView::TaskGraphView(const MString& viewName)
	: BaseWidget()
{
	m_strViewName = viewName;
}

int GetDepthTable(MTaskGraph* pTaskGraph, std::map<MTaskNode*, int>& output)
{
	int nMaxDepth = 0;
	auto vNodes = pTaskGraph->GetFinalNodes();

	for (MTaskNode* pNode : vNodes)
	{
		output[pNode] = 0;
	}

	while(!vNodes.empty())
	{
		MTaskNode* pNode = vNodes.back();
		vNodes.pop_back();

		for (size_t nInputIdx = 0; nInputIdx < pNode->GetInputSize(); ++nInputIdx)
		{
			MTaskNode* pPrevNode = pNode->GetInput(nInputIdx)->GetLinkedNode();

			if (output.find(pPrevNode) == output.end())
			{
				output[pPrevNode] = output[pNode] + 1;
			}
			else if (output[pPrevNode] < output[pNode] + 1)
			{
				output[pPrevNode] = output[pNode] + 1;
			}

			vNodes.push_back(pPrevNode);

			if (nMaxDepth < output[pPrevNode])
			{
				nMaxDepth = output[pPrevNode];
			}
		}
	}

	return nMaxDepth;
}

struct DrawNode
{
	ImVec2 pos{};
	MTaskNode* pTaskNode = nullptr;
};

void TaskGraphView::Render()
{
	if (!m_pTaskGraph)
	{
		return;
	}

	std::map<MTaskNode*, int> tDepthTable;
	const int nMaxDepth = GetDepthTable(m_pTaskGraph, tDepthTable);
	std::vector<int> vTaskColumn(nMaxDepth + 1, 0);

	auto vAllNodes = m_pTaskGraph->GetAllNodes();

	ImNodes::BeginNodeEditor();

	for (auto& pNode : vAllNodes)
	{
		const int imNodeId = static_cast<int>(reinterpret_cast<std::intptr_t>(pNode));
		(ImNodes::BeginNode(imNodeId));
		{
			//first initialize position.
			ImVec2 size = ImNodes::GetNodeDimensions(imNodeId);
			if (size.x + size.y <= MGlobal::M_FLOAT_BIAS)
			{
				ImVec2 initialPosition;
				initialPosition.x = -tDepthTable[pNode] * 300;
				initialPosition.y = vTaskColumn[tDepthTable[pNode]] * 200;
				ImNodes::SetNodeEditorSpacePos(imNodeId, initialPosition);

				vTaskColumn[tDepthTable[pNode]]++;
			}

			//title
			ImNodes::BeginNodeTitleBar();
			ImGui::TextUnformatted(pNode->GetNodeName().ToString().c_str());
			ImNodes::EndNodeTitleBar();

			//input
			for (size_t nIdx = 0; nIdx < pNode->GetInputSize(); ++nIdx)
			{
				auto pNodeInput = pNode->GetInput(nIdx);
				ImNodes::BeginInputAttribute( static_cast<int>(reinterpret_cast<std::intptr_t>(pNodeInput)));
				ImGui::Text("%s", pNodeInput->GetName().c_str());
				ImNodes::EndInputAttribute();
			}

#if MORTY_DEBUG
			//Context
			ImGui::Text("avg time: %f", (static_cast<float>(pNode->m_nDebugTime) / 1000.0f));
#endif
			//output
			for (size_t nIdx = 0; nIdx < pNode->GetOutputSize(); ++nIdx)
			{
				auto pNodeOutput = pNode->GetOutput(nIdx);
				ImNodes::BeginOutputAttribute(static_cast<int>(reinterpret_cast<std::intptr_t>(pNodeOutput)));
				ImGui::Text("%s", pNodeOutput->GetName().c_str());
				ImNodes::EndOutputAttribute();
			}
		}
		ImNodes::EndNode();
	}

	//link
	for (auto& pNode : vAllNodes)
	{
		for (size_t nInputIdx = 0; nInputIdx < pNode->GetInputSize(); ++nInputIdx)
		{
			auto pInput = pNode->GetInput(nInputIdx);
			auto pOutput = pInput->GetLinkedOutput();
			if (!pOutput)
			{
				continue;
			}

			const int inputId = static_cast<int>(reinterpret_cast<std::intptr_t>(pInput));
			const int outputId = static_cast<int>(reinterpret_cast<std::intptr_t>(pOutput));

			ImNodes::Link(inputId + outputId, outputId, inputId);
		}
	}

	
	ImNodes::EndNodeEditor();

}

void TaskGraphView::Initialize(MainEditor* pMainEditor)
{
	BaseWidget::Initialize(pMainEditor);

}

void TaskGraphView::Release()
{

}

void TaskGraphView::SetTaskGraph(MTaskGraph* pGraph)
{
	m_pTaskGraph = pGraph;
}
