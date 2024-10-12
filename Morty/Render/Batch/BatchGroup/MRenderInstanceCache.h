#pragma once

#include "Utility/MGlobal.h"
#include "Utility/MBounds.h"
#include "Utility/MIDPool.h"
#include "Utility/MMemoryPool.h"

namespace morty
{

template<typename KEY_TYPE, typename VALUE_TYPE> class MORTY_API MRenderInstanceCache
{
public:
    struct CacheItem {
        bool       bValid = false;
        VALUE_TYPE value;
    };

public:
    size_t                        AddItem(const KEY_TYPE& key, const VALUE_TYPE& value);

    void                          RemoveItem(const KEY_TYPE& key);

    bool                          HasItem(const KEY_TYPE& key);

    size_t                        GetItemIdx(const KEY_TYPE& key);

    VALUE_TYPE*                   FindItem(const KEY_TYPE& key);


    const std::vector<CacheItem>& GetItems() const { return m_item; }

    std::vector<CacheItem>        m_item;
    std::map<KEY_TYPE, size_t>    m_table;
    MRepeatIDPool<size_t>         m_itemPool;
};

template<typename KEY_TYPE, typename VALUE_TYPE>
inline size_t MRenderInstanceCache<KEY_TYPE, VALUE_TYPE>::AddItem(const KEY_TYPE& key, const VALUE_TYPE& value)
{
    const auto findResult = m_table.find(key);
    if (findResult != m_table.end()) { return findResult->second; }

    const size_t nInstanceIdx = m_itemPool.GetNewID();
    if (nInstanceIdx >= m_item.size()) { m_item.resize(nInstanceIdx + 1); }

    m_item[nInstanceIdx].bValid = true;
    m_item[nInstanceIdx].value  = value;
    m_table[key]                = nInstanceIdx;

    return nInstanceIdx;
}

template<typename KEY_TYPE, typename VALUE_TYPE>
inline void MRenderInstanceCache<KEY_TYPE, VALUE_TYPE>::RemoveItem(const KEY_TYPE& key)
{
    const auto findResult = m_table.find(key);
    if (findResult == m_table.end())
    {
        MORTY_ASSERT(false);
        return;
    }

    const size_t nIdx = findResult->second;
    auto&        item = m_item[nIdx];
    item.bValid       = false;

    m_itemPool.RecoveryID(nIdx);
    m_table.erase(findResult);
}

template<typename KEY_TYPE, typename VALUE_TYPE>
inline bool MRenderInstanceCache<KEY_TYPE, VALUE_TYPE>::HasItem(const KEY_TYPE& key)
{
    const auto findResult = m_table.find(key);
    return findResult != m_table.end();
}

template<typename KEY_TYPE, typename VALUE_TYPE>
inline size_t MRenderInstanceCache<KEY_TYPE, VALUE_TYPE>::GetItemIdx(const KEY_TYPE& key)
{
    const auto findResult = m_table.find(key);
    if (findResult == m_table.end()) { return MGlobal::M_INVALID_INDEX; }

    return findResult->second;
}

template<typename KEY_TYPE, typename VALUE_TYPE>
inline VALUE_TYPE* MRenderInstanceCache<KEY_TYPE, VALUE_TYPE>::FindItem(const KEY_TYPE& key)
{
    const auto findResult = m_table.find(key);
    if (findResult == m_table.end()) { return nullptr; }

    const size_t nIdx = findResult->second;
    auto&        item = m_item[nIdx];
    return &item.value;
}

}// namespace morty