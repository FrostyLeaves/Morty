#include "MComponent.h"

M_OBJECT_IMPLEMENT(MComponent, MTypedClass)

#include "MNode.h"

MComponent::MComponent()
	: MTypedClass()
	, m_pOwnerNode(nullptr)
{

}

MComponent::~MComponent()
{

}

void MComponent::SetOwnerNode(MNode* pNode)
{
	m_pOwnerNode = pNode;
}

MScene* MComponent::GetScene()
{
	if (!m_pOwnerNode)
		return nullptr;

	return m_pOwnerNode->GetScene();
}

MEngine* MComponent::GetEngine()
{
	if (!m_pOwnerNode)
		return nullptr;

	return m_pOwnerNode->GetEngine();
}

void MComponent::SendComponentNotify(const MString& strSignalName)
{
	MNode* pOwnerNode = GetOwnerNode();
	if (!pOwnerNode)
		return;

	pOwnerNode->SendComponentNotify(strSignalName);
}

void MComponent::WriteToStruct(MStruct& srt)
{
}
