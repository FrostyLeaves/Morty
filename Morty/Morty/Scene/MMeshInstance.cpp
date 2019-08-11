#include "MMeshInstance.h"
#include "MModelResource.h"

MMeshInstance::MMeshInstance()
	: M3DNode()
{

}

MMeshInstance::~MMeshInstance()
{

}

bool MMeshInstance::Load(const MResource* pResource)
{
	if (m_pResource = dynamic_cast<const MModelResource*>(pResource))
	{
		//Do smoething.
		return true;
	}

	return false;
}

void MMeshInstance::OnTick(const float& fDelta)
{

}

void MMeshInstance::Render()
{

}
