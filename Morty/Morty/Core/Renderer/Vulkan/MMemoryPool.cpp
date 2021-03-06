#include "MMemoryPool.h"

#include <algorithm>

MMemoryPool::MMemoryPool(const uint32_t& unPoolSize)
	: m_unBufferMemorySize(0)
	, m_vFreeMemory()
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
	auto iter = std::lower_bound(m_vFreeMemory.begin(), m_vFreeMemory.end(), info);

	if (m_vFreeMemory.end() - iter <= 1)
	{
		MemoryInfo& back = m_vFreeMemory.back();
		if (back.begin + back.size == info.begin)
			back.size += info.size;
		else
			m_vFreeMemory.push_back(info);
	}
	else if (iter == m_vFreeMemory.begin())
	{
		MemoryInfo& front = m_vFreeMemory.front();
		if (info.begin + info.size == front.begin)
		{
			front.begin = info.begin;
			front.size -= info.size;
		}
		else
			m_vFreeMemory.insert(m_vFreeMemory.begin(), info);
	}
	else
	{
		MemoryInfo& prev = *(iter - 1);
		MemoryInfo& next = *(iter + 1);
		if (prev.begin + prev.size == info.begin)
		{
			prev.size += info.size;
		}
		else if (info.begin + info.size == next.begin)
		{
			next.begin -= info.size;
			next.size += info.size;
		}
		else
			m_vFreeMemory.insert(iter, info);
	}
}
