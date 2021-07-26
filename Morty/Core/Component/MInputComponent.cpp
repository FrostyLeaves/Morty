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
		return m_funcInputCallback(pEvent, pViewport);

	return false;
}

void MInputComponent::WriteToStruct(MStruct& srt)
{
	Super::WriteToStruct(srt);
}

void MInputComponent::ReadFromStruct(const MStruct& srt)
{
	Super::ReadFromStruct(srt);
}
