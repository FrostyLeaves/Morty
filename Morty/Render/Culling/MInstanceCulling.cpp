#include "MInstanceCulling.h"
#include "Batch/BatchGroup/MInstanceBatchGroup.h"

bool MMaterialTypeFilter::Filter(const std::shared_ptr<MMaterial>& material) const
{
	return material->GetMaterialType() == m_eMaterialType;
}

bool MMaterialMacroDefineFilter::Filter(const std::shared_ptr<MMaterial>& material) const
{
	for (const auto& [name, defined] : m_definedMacro)
	{
		if (defined != material->GetShaderMacro().HasMacro(name))
		{
			return false;
		}
	}

	return true;
}