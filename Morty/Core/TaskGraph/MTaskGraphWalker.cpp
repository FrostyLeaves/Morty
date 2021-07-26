#include "MTaskGraphWalker.h"

#include "MEngine.h"
#include "MFunction.h"
#include "MTaskGraph.h"
#include "MThreadPool.h"

MTaskGraphWalker::MTaskGraphWalker()
	: m_vWaitTask()
	, m_vActiveTask()
	, m_tNodeState()
	, m_taskStatehMutex()
{

}

MTaskGraphWalker::~MTaskGraphWalker()
{

}

void MTaskGraphWalker::operator()(MTaskGraph* pTaskGraph)
{
	if (!pTaskGraph->IsValid())
		return;

	if (pTaskGraph->NeedCompile() && !pTaskGraph->Compile())
		return;


	MEngine* pEngine = pTaskGraph->GetEngine();
	MThreadPool* pThreadPool = pEngine->GetThreadPool();

	const std::vector<MTaskNode*>& vNodes = pTaskGraph->GetStartNodes();

	for (MTaskNode* pNode : vNodes)
	{
		m_tNodeState[pNode] = METaskState::Active;
		m_vWaitTask.push(pNode);
	}

	MTaskNode* pTaskNode = nullptr;
	while (!m_vWaitTask.empty() || !m_vActiveTask.empty())
	{
		{
			std::unique_lock<std::mutex> lck(m_taskStatehMutex);

			if (m_vWaitTask.empty())
				continue;

			pTaskNode = m_vWaitTask.front();
			m_vWaitTask.pop();

			if (!pTaskNode)
				continue;

			UNION_PUSH_BACK_VECTOR(m_vActiveTask, pTaskNode);
		}

		m_tNodeState[pTaskNode] = METaskState::Active;
		pThreadPool->AddWork(CreateThreadWork(pTaskNode));
	}
}

bool MTaskGraphWalker::CheckNodeActive(MTaskNode* pNode)
{
	if (m_tNodeState[pNode] != METaskState::Wait)
		return false;

	for (size_t nInputIdx = 0; nInputIdx < pNode->GetInputSize(); ++nInputIdx)
	{
		MTaskNodeInput* pInput = pNode->GetInput(nInputIdx);

		if (MTaskNode* pDependNode = pInput->GetLinkedNode())
		{
			if (m_tNodeState[pDependNode] != METaskState::Finish)
			{
				return false;
			}
		}
	}

	return true;
}

MThreadWork MTaskGraphWalker::CreateThreadWork(MTaskNode* pTaskNode)
{
	MThreadWork work;
	work.eThreadType = pTaskNode->GetThreadType();
	work.funcWorkFunction = [=]() {

		pTaskNode->Run();

		{
			std::unique_lock<std::mutex> lck(m_taskStatehMutex);
			OnTaskFinishedCallback(pTaskNode);
		}
	};

	return work;
}

void MTaskGraphWalker::OnTaskFinishedCallback(MTaskNode* pTaskNode)
{
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
