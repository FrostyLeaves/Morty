#include "MMemoryPool.h"

using namespace morty;

MMemoryPool::MMemoryPool(const size_t& unPoolSize)
    : m_maxMemorySize(unPoolSize)
{

    MemoryInfo info;
    info.begin = 0;
    info.size  = unPoolSize;

    m_freeMemory.push_back(info);
}

MMemoryPool::~MMemoryPool() {}

bool MMemoryPool::AllowMemory(const size_t& unVariantSize, MemoryInfo& info)
{
    auto biggestIter = m_freeMemory.begin();
    auto bestIter    = m_freeMemory.end();
    for (auto iter = m_freeMemory.begin(); iter != m_freeMemory.end(); ++iter)
    {
        if (iter->size == unVariantSize)
        {
            bestIter = iter;
            break;
        }

        if (iter->size > biggestIter->size) biggestIter = iter;
    }

    if (bestIter == m_freeMemory.end() && biggestIter->size >= unVariantSize)
        bestIter = biggestIter;

    if (bestIter == m_freeMemory.end()) { return false; }

    if (bestIter->size == unVariantSize)
    {
        info = *bestIter;
        m_freeMemory.erase(bestIter);
    }
    else if (bestIter->size > unVariantSize)
    {
        info      = *bestIter;
        info.size = unVariantSize;

        bestIter->begin += unVariantSize;
        bestIter->size -= unVariantSize;
    }
    else { return false; }

    return true;
}

void MMemoryPool::FreeMemory(MemoryInfo& info)
{
    if (m_freeMemory.empty())
    {
        m_freeMemory.push_back(info);
        return;
    }

    auto findResult = std::lower_bound(m_freeMemory.begin(), m_freeMemory.end(), info);

    if (m_freeMemory.end() == findResult)
    {
        MemoryInfo& back = m_freeMemory.back();
        if (back.begin + back.size == info.begin) back.size += info.size;
        else
            m_freeMemory.push_back(info);
    }
    else if (m_freeMemory.begin() == findResult)
    {
        MemoryInfo& front = m_freeMemory.front();
        if (info.begin + info.size == front.begin)
        {
            front.begin = info.begin;
            front.size += info.size;
        }
        else
            m_freeMemory.insert(m_freeMemory.begin(), info);
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
                m_freeMemory.erase(findResult);
            }
        }
        else if (info.begin + info.size == findResult->begin)
        {
            findResult->begin = info.begin;
            findResult->size += info.size;
        }
        else { m_freeMemory.insert(findResult, info); }
    }
}

void MMemoryPool::ResizeMemory(const size_t& nPoolSize)
{
    if (nPoolSize <= m_maxMemorySize)
    {
        MORTY_ASSERT(nPoolSize > m_maxMemorySize);
        return;
    }

    if (m_freeMemory.empty())
    {
        MORTY_ASSERT(!m_freeMemory.empty());
        m_maxMemorySize = nPoolSize;
        return;
    }

    const size_t nAllowSize = nPoolSize - m_maxMemorySize;

    MemoryInfo&  back = m_freeMemory.back();
    if (back.begin + back.size == m_maxMemorySize) { back.size += nAllowSize; }
    else
    {
        MemoryInfo back;
        back.begin = m_maxMemorySize;
        back.size  = nAllowSize;
        m_freeMemory.push_back(back);
    }

    m_maxMemorySize = nPoolSize;
}
