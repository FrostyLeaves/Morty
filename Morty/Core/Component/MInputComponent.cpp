#include "Component/MInputComponent.h"

using namespace morty;

MORTY_CLASS_IMPLEMENT(MInputComponent, MComponent)

MInputComponent::MInputComponent()
    : MComponent()
    , m_funcInputCallback(nullptr)
{}

MInputComponent::~MInputComponent() {}

bool MInputComponent::Input(MInputEvent* pEvent, MViewport* pViewport)
{
    if (m_funcInputCallback) return m_funcInputCallback(this, pEvent, pViewport);

    return false;
}
