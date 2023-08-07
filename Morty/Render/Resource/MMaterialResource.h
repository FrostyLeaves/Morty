/**
 * @File         MMaterialResource
 * 
 * @Created      2019-09-01 15:25:21
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Material/MMaterial.h"

class MORTY_API MMaterialResource : public MMaterial
{
public:
	MORTY_CLASS(MMaterialResource);

	std::shared_ptr<MMaterial> GetMaterial() const;

	bool Load(std::unique_ptr<MResourceData>&& pResourceData) override;
	virtual bool SaveTo(std::unique_ptr<MResourceData>& pResourceData) override;

private:

	std::unique_ptr<MResourceData> m_pResourceData = nullptr;

};
