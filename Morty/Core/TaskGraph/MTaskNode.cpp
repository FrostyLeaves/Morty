#include "TaskGraph/MTaskNode.h"
#include "TaskGraph/MTaskGraph.h"
#include "TaskGraph/MTaskNodeInput.h"
#include "TaskGraph/MTaskNodeOutput.h"
#include "Utility/MTimer.h"

using namespace morty;

MORTY_CLASS_IMPLEMENT(MTaskNode, MTypeClass)


MTaskNode::~MTaskNode()
{
	for (MTaskNodeInput* pInput : m_vInput)
	{
		delete pInput;
	}
	m_vInput.clear();

	for (MTaskNodeOutput* pOutput : m_vOutput)
	{
		delete pOutput;
	}
	m_vOutput.clear();
}

void MTaskNode::AppendInput(MTaskNodeInput* pInput)
{
	pInput->pGraphNode = this;
	pInput->m_unIndex = m_vInput.size();
	m_vInput.push_back(pInput);
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

void MTaskNode::ConnectTo(MTaskNode* pNextNode)
{
	MTaskNodeOutput* pOutput = GetOutputSize() ? GetOutput(0) : AppendOutput();
	MTaskNodeInput* pInput = pNextNode->AppendInput();

	pOutput->LinkTo(pInput);
}

void MTaskNode::DisconnectTo(MTaskNode* pNextNode)
{
    for (auto iter = m_vOutput.begin(); iter != m_vOutput.end(); ++iter)
    {
		MTaskNodeOutput* pOutput = *iter;
		auto inputs = pOutput->GetLinkedInputs();
		for (MTaskNodeInput* pInput : inputs)
		{
		    if (pInput->GetTaskNode() == pNextNode)
		    {
				pOutput->UnLink(pInput);
		    }
		}
    }
}


void MTaskNode::DisconnectAll()
{
    for (MTaskNodeInput* pInput : m_vInput)
    {
        if (auto pPrevNode = pInput->GetLinkedNode())
        {
			pPrevNode->DisconnectTo(this);
        }

		delete pInput;
    }
	m_vInput.clear();

	for (MTaskNodeOutput* pOutput : m_vOutput)
	{
		auto inputs = pOutput->GetLinkedInputs();
		for (MTaskNodeInput* pInput : inputs)
		{
			pOutput->UnLink(pInput);
		}

		delete pOutput;
	}
	m_vOutput.clear();
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
#if MORTY_DEBUG
	auto start = MTimer::GetCurTime();
#endif
	if (m_funcTaskFunction)
	{
		m_funcTaskFunction(this);
	}

#if MORTY_DEBUG
	auto end = MTimer::GetCurTime();
	m_nDebugTime = ((m_nDebugTime * 29) + (end - start)) / 30;
#endif
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
