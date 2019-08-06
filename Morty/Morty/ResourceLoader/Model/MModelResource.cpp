#include "MModelResource.h"
#include "MModel.h"

MModelResource::MModelResource()
: MResource()
, m_pModelTemplate(nullptr)
{
    m_pModelTemplate = new MModel();
}

MModelResource::~MModelResource()
{
    
}
