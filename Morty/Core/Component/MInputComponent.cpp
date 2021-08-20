#include "MInputComponent.h"

MORTY_CLASS_IMPLEMENT(MInputComponent, MComponent)

MInputComponent::MInputComponent()
	: MComponent()
	, m_funcInputCallback(nullptr)
{
}

MInputComponent::~MInputComponent()
{

}

bool MInputComponent::Input(MInputEvent* pEvent, MViewport* pViewport)
{
	if (m_funcInputCallback)
		return m_funcInputCallback(this, pEvent, pViewport);

	return false;
}

void MInputComponent::WriteToStruct(MStruct& srt, MComponentRefTable& refTable)
{
	Super::WriteToStruct(srt, refTable);
}

void MInputComponent::ReadFromStruct(const MStruct& srt, MComponentRefTable& refTable)
{
	Super::ReadFromStruct(srt, refTable);
}
