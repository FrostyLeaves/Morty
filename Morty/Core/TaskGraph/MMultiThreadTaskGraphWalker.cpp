#include "MMultiThreadTaskGraphWalker.h"

#include "Engine/MEngine.h"
#include "Utility/MFunction.h"
#include "TaskGraph/MTaskGraph.h"
#include "Thread/MThreadPool.h"

MMultiThreadTaskGraphWalker::MMultiThreadTaskGraphWalker(MThreadPool* pThreadPool)
	: m_pThreadPool(pThreadPool)
{

}

void MMultiThreadTaskGraphWalker::operator()(MTaskGraph* pTaskGraph)
{
	MORTY_ASSERT(m_pThreadPool);

    if (!m_pThreadPool)
	{
		return;
	}

	if (pTaskGraph->NeedCompile() && !pTaskGraph->Compile())
	{
		return;
	}
	const std::vector<MTaskNode*>& vNodes = pTaskGraph->GetStartNodes();

	for (MTaskNode* pNode : vNodes)
	{
		m_tNodeState[pNode] = METaskState::Active;
		m_vWaitTask.push(pNode);
	}

	MTaskNode* pTaskNode = nullptr;
	while (true)
	{
		{
			std::unique_lock<std::mutex> lck(m_taskStatehMutex);

			if (m_vWaitTask.empty() && m_vActiveTask.empty())
				break;

			if (m_vWaitTask.empty())
				continue;

			pTaskNode = m_vWaitTask.front();
			m_vWaitTask.pop();

			if (!pTaskNode)
				continue;

			UNION_PUSH_BACK_VECTOR(m_vActiveTask, pTaskNode);
		}

		m_tNodeState[pTaskNode] = METaskState::Active;
		m_pThreadPool->AddWork(CreateThreadWork(pTaskNode));
	}
}

bool MMultiThreadTaskGraphWalker::CheckNodeActive(MTaskNode* pNode) const
{
	const auto findState = m_tNodeState.find(pNode);
	if (findState != m_tNodeState.end() && findState->second != METaskState::Wait)
		return false;

	for (size_t nInputIdx = 0; nInputIdx < pNode->GetInputSize(); ++nInputIdx)
	{
		MTaskNodeInput* pInput = pNode->GetInput(nInputIdx);

		if (MTaskNode* pDependNode = pInput->GetLinkedNode())
		{
			const auto findDependState = m_tNodeState.find(pDependNode);
			if (findDependState == m_tNodeState.end() || findDependState->second != METaskState::Finish)
			{
				return false;
			}
		}
	}

	return true;
}

MThreadWork MMultiThreadTaskGraphWalker::CreateThreadWork(MTaskNode* pTaskNode)
{
	MThreadWork work;
	work.eThreadType = pTaskNode->GetThreadType();
	work.funcWorkFunction = [=]() {
		pTaskNode->Run();
		OnTaskFinishedCallback(pTaskNode);
	};

	return work;
}

void MMultiThreadTaskGraphWalker::OnTaskFinishedCallback(MTaskNode* pTaskNode)
{
	std::unique_lock<std::mutex> lck(m_taskStatehMutex);

	m_tNodeState[pTaskNode] = METaskState::Finish;

	for (size_t i = 0; i < pTaskNode->GetOutputSize(); ++i)
	{
		MTaskNodeOutput* pOutput = pTaskNode->GetOutput(i);
		const auto& vInputs = pOutput->GetLinkedInputs();
		for (MTaskNodeInput* pInput : vInputs)
		{
			MTaskNode* pNextNode = pInput->GetTaskNode();

			if (CheckNodeActive(pNextNode))
			{
				m_tNodeState[pNextNode] = METaskState::Active;
				m_vWaitTask.push(pNextNode);
			}
		}
	}

	ERASE_FIRST_VECTOR(m_vActiveTask, pTaskNode);
}
