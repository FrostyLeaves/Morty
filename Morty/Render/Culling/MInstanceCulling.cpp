#include "MInstanceCulling.h"
#include "Batch/BatchGroup/MInstanceBatchGroup.h"

bool MMaterialTypeFilter::Filter(const std::shared_ptr<MMaterial>& material) const
{
	return material->GetMaterialType() == m_eMaterialType;
}