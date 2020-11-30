#include "MIPostProcessWork.h"

M_OBJECT_IMPLEMENT(MIPostProcessWork, MObject)

MIPostProcessWork::MIPostProcessWork()
    : MObject()
{
}

MIPostProcessWork::~MIPostProcessWork()
{
}

void MIPostProcessWork::OnDelete()
{
    Release();

    Super::OnDelete();
}

