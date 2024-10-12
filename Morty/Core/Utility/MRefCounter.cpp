#include "MRefCounter.h"

using namespace morty;

void MRefCounter::OnReferenceZero() { delete this; }
