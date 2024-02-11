#pragma once

#include "MBoundingCulling.h"
#include "MInstanceCulling.h"

MORTY_SPACE_BEGIN

class MInstanceBatchGroup;

class MORTY_API MBoundingBoxFilter : public IMeshInstanceFilter
{
public:

    bool Filter(const MMeshInstanceRenderProxy* instance) const override;

	MBoundsAABB m_bounds;
};


class MORTY_API MBoundingBoxCulling
    : public MBoundingCulling
{
public:

    void Initialize(MEngine* pEngine) override;
    void Release() override;

    void SetBounds(const MBoundsAABB& bounds);

private:
    std::shared_ptr<MBoundingBoxFilter> m_pBoundsFilter = nullptr;
};

MORTY_SPACE_END