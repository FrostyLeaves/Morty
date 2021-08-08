#include "MTaskNode.h"
#include "MTaskGraph.h"
#include "MTaskNodeInput.h"
#include "MTaskNodeOutput.h"

MORTY_CLASS_IMPLEMENT(MTaskNode, MTypeClass)


MTaskNode::MTaskNode()
	: m_funcTaskFunction(nullptr)
	, m_eThreadType(METhreadType::EAny)
{

}

MTaskNode::~MTaskNode()
{

}

MTaskNodeInput* MTaskNode::AppendInput()
{
	MTaskNodeInput* pInput = new MTaskNodeInput();
	pInput->pGraphNode = this;
	pInput->m_unIndex = m_vInput.size();
	m_vInput.push_back(pInput);

	return pInput;
}

void MTaskNode::AppendOutput(MTaskNodeOutput* pOutput)
{
	pOutput->pGraphNode = this;
	pOutput->m_unIndex = m_vOutput.size();

	m_vOutput.push_back(pOutput);
}

MTaskNodeInput* MTaskNode::GetInput(const size_t& nInputIdx)
{
	if (nInputIdx < m_vInput.size())
		return m_vInput[nInputIdx];

	return nullptr;
}

MTaskNodeOutput* MTaskNode::GetOutput(const size_t& nOutputIdx)
{
	if (nOutputIdx < m_vOutput.size())
		return m_vOutput[nOutputIdx];

	return nullptr;
}

MEngine* MTaskNode::GetEngine()
{
	if (!m_pGraph)
		return nullptr;

	return m_pGraph->GetEngine();
}

bool MTaskNode::IsStartNode()
{
	for (MTaskNodeInput* pInput : m_vInput)
	{
		if (pInput->GetLinkedNode())
			return false;
	}

	return true;
}

bool MTaskNode::IsFinalNode()
{
	for (MTaskNodeOutput* pOutput : m_vOutput)
	{
		if (!pOutput->GetLinkedInputs().empty())
			return false;
	}

	return true;
}

void MTaskNode::Run()
{
	if (m_funcTaskFunction)
	{
		m_funcTaskFunction(this);
	}
}

void MTaskNode::OnCreated()
{

}

void MTaskNode::OnCompile()
{

}

void MTaskNode::OnDelete()
{

}
