#include "MRefCounter.h"

void MRefCounter::OnReferenceZero()
{
	delete this;
}
