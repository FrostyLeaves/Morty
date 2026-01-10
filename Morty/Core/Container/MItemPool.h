/**
 * @File         MItemPool
 * 
 * @Created      2026-01-06
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"
#include "Utility/MIDPool.h"

#include <optional>
#include <utility>
#include <vector>

namespace morty
{

template<typename TYPE> class MORTY_API MItemPool
{
public:
    using Id = size_t;

    template<typename... Args> Id Emplace(Args&&... args)
    {
        Id id = m_idPool.AllocateID();
        if (id >= m_items.size()) { m_items.resize(id + 1); }

        Slot& slot = m_items[id];
        MORTY_ASSERT(!slot.value.has_value());
        slot.value.emplace(std::forward<Args>(args)...);
        ++m_aliveCount;

        return id;
    }

    Id Allocate() { return Emplace(); }

    void Release(const Id id)
    {
        if (id >= m_items.size()) { return; }

        Slot& slot = m_items[id];
        if (!slot.value.has_value()) { return; }

        slot.value.reset();
        m_idPool.FreeID(id);
        --m_aliveCount;
    }

    TYPE* Get(const Id id)
    {
        if (id >= m_items.size()) { return nullptr; }

        Slot& slot = m_items[id];
        if (!slot.value.has_value()) { return nullptr; }

        return &slot.value.value();
    }

    const TYPE* Get(const Id id) const
    {
        if (id >= m_items.size()) { return nullptr; }

        const Slot& slot = m_items[id];
        if (!slot.value.has_value()) { return nullptr; }

        return &slot.value.value();
    }

    bool IsAlive(const Id id) const
    {
        if (id >= m_items.size()) { return false; }
        return m_items[id].value.has_value();
    }

    size_t Size() const { return m_aliveCount; }

    void Clear()
    {
        m_items.clear();
        m_aliveCount = 0;
        m_idPool     = MReusableIDPool<Id>();
    }

private:
    struct Slot {
        std::optional<TYPE> value;
    };

    std::vector<Slot>   m_items;
    MReusableIDPool<Id> m_idPool;
    size_t              m_aliveCount = 0;
};

}// namespace morty
