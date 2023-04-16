#ifndef _M_RENDER_INSTANCE_CACHE_H_
#define _M_RENDER_INSTANCE_CACHE_H_

#include "Utility/MGlobal.h"
#include "Utility/MBounds.h"
#include "Utility/MIDPool.h"
#include "Utility/MMemoryPool.h"


template<typename KEY_TYPE, typename VALUE_TYPE>
class MORTY_API MRenderInstanceCache
{
public:
	struct CacheItem
	{
		bool bValid = false;
		VALUE_TYPE value;
	};
public:

	size_t AddItem(const KEY_TYPE& key, const VALUE_TYPE& value);
	void RemoveItem(const KEY_TYPE& key);
	bool HasItem(const KEY_TYPE& key);
	VALUE_TYPE* FindItem(const KEY_TYPE& key);


	const std::vector<CacheItem>& GetItems() const { return m_vItem; }

	std::vector<CacheItem> m_vItem;
	std::map<KEY_TYPE, size_t> m_tTable;
	MRepeatIDPool<size_t> m_itemPool;

};


#endif

template<typename KEY_TYPE, typename VALUE_TYPE>
inline size_t MRenderInstanceCache<KEY_TYPE, VALUE_TYPE>::AddItem(const KEY_TYPE& key, const VALUE_TYPE& value)
{
	const auto findResult = m_tTable.find(key);
	if (findResult != m_tTable.end())
	{
		return findResult->second;
	}

	const size_t nInstanceIdx = m_itemPool.GetNewID();
	if (nInstanceIdx >= m_vItem.size())
	{
		m_vItem.resize(nInstanceIdx + 1);
	}

	m_vItem[nInstanceIdx].bValid = true;
	m_vItem[nInstanceIdx].value = value;
	m_tTable[key] = nInstanceIdx;

	return nInstanceIdx;
}

template<typename KEY_TYPE, typename VALUE_TYPE>
inline void MRenderInstanceCache<KEY_TYPE, VALUE_TYPE>::RemoveItem(const KEY_TYPE& key)
{
	const auto findResult = m_tTable.find(key);
	if (findResult == m_tTable.end())
	{
		MORTY_ASSERT(false);
		return;
	}

	const size_t nIdx = findResult->second;
	auto& item = m_vItem[nIdx];
	item.bValid = false;

	m_itemPool.RecoveryID(nIdx);
	m_tTable.erase(findResult);
}

template<typename KEY_TYPE, typename VALUE_TYPE>
inline bool MRenderInstanceCache<KEY_TYPE, VALUE_TYPE>::HasItem(const KEY_TYPE& key)
{
	const auto findResult = m_tTable.find(key);
	return findResult != m_tTable.end();
}

template<typename KEY_TYPE, typename VALUE_TYPE>
inline VALUE_TYPE* MRenderInstanceCache<KEY_TYPE, VALUE_TYPE>::FindItem(const KEY_TYPE& key)
{
	const auto findResult = m_tTable.find(key);
	if (findResult == m_tTable.end())
	{
		return nullptr;
	}

	const size_t nIdx = findResult->second;
	auto& item = m_vItem[nIdx];
	return &item.value;
}
