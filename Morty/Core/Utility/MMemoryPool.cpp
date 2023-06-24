#include "MMemoryPool.h"

#include <algorithm>

MMemoryPool::MMemoryPool(const size_t& unPoolSize)
	: m_vFreeMemory()
    , m_nMaxMemorySize(unPoolSize)
{

	MemoryInfo info;
	info.begin = 0;
	info.size = unPoolSize;

	m_vFreeMemory.push_back(info);
}

MMemoryPool::~MMemoryPool()
{

}

bool MMemoryPool::AllowMemory(const uint32_t& unVariantSize, MemoryInfo& info)
{
	auto biggestIter = m_vFreeMemory.begin();
	auto bestIter = m_vFreeMemory.end();
	for (auto iter = m_vFreeMemory.begin(); iter != m_vFreeMemory.end(); ++iter)
	{
		if (iter->size == unVariantSize)
		{
			bestIter = iter;
			break;
		}

		if (iter->size > biggestIter->size)
			biggestIter = iter;
	}

	if (bestIter == m_vFreeMemory.end() && biggestIter->size >= unVariantSize)
		bestIter = biggestIter;

	if (bestIter == m_vFreeMemory.end())
	{
		return false;
	}

	if (bestIter->size == unVariantSize)
	{
		info = *bestIter;
		m_vFreeMemory.erase(bestIter);

	}
	else if (bestIter->size > unVariantSize)
	{
		info = *bestIter;
		info.size = unVariantSize;

		bestIter->begin += unVariantSize;
		bestIter->size -= unVariantSize;
	}
	else
	{
		return false;
	}

	return true;
}

void MMemoryPool::FreeMemory(MemoryInfo& info)
{
	if (m_vFreeMemory.empty())
	{
		m_vFreeMemory.push_back(info);
		return;
	}

	auto findResult = std::lower_bound(m_vFreeMemory.begin(), m_vFreeMemory.end(), info);

	if (m_vFreeMemory.end() == findResult)
	{
		MemoryInfo& back = m_vFreeMemory.back();
		if (back.begin + back.size == info.begin)
			back.size += info.size;
		else
			m_vFreeMemory.push_back(info);
	}
	else if (m_vFreeMemory.begin() == findResult)
	{
		MemoryInfo& front = m_vFreeMemory.front();
		if (info.begin + info.size == front.begin)
		{
			front.begin = info.begin;
			front.size += info.size;
		}
		else
			m_vFreeMemory.insert(m_vFreeMemory.begin(), info);
	}
	else
	{
		auto prev = findResult - 1;
		
		if (prev->begin + prev->size == info.begin)
		{
			prev->size += info.size;

			if (prev->begin + prev->size == findResult->begin)
			{
				prev->size += findResult->size;
				m_vFreeMemory.erase(findResult);
			}
		}
		else if (info.begin + info.size == findResult->begin)
		{
			findResult->begin = info.begin;
			findResult->size += info.size;
		}
		else
		{
			m_vFreeMemory.insert(findResult, info); 
		}
	}
}

void MMemoryPool::ResizeMemory(const size_t& nPoolSize)
{
    if (nPoolSize <= m_nMaxMemorySize)
    {
		MORTY_ASSERT(nPoolSize > m_nMaxMemorySize);
		return;
    }

	if (m_vFreeMemory.empty())
	{
		MORTY_ASSERT(!m_vFreeMemory.empty());
		m_nMaxMemorySize = nPoolSize;
		return;
	}

	const size_t nAllowSize = nPoolSize - m_nMaxMemorySize;

	MemoryInfo& back = m_vFreeMemory.back();
	if (back.begin + back.size == m_nMaxMemorySize)
	{
		back.size += nAllowSize;
	}
	else
	{
		MemoryInfo back;
		back.begin = m_nMaxMemorySize;
		back.size = nAllowSize;
		m_vFreeMemory.push_back(back);
	}

	m_nMaxMemorySize = nPoolSize;
}