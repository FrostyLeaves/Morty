#include "MStringId.h"
#include "Thread/MThreadPool.h"

thread_local static std::vector<std::string> ClashStringTable;

MStringId::MStringId(std::string_view strview)
{
    m_string = std::make_shared<MString>(strview);

    m_hash = std::hash<std::string>{}(*m_string);

    m_clashIndex = std::make_shared<std::array<size_t, MGlobal::M_MAX_THREAD_NUM>>();
}

bool MStringId::operator ==(const MStringId& other) const
{
    if (m_hash != other.m_hash)
    {
        return false;
    }

    return GetClashIndex() == other.GetClashIndex();
}

bool MStringId::operator <(const MStringId& other) const
{
    if (m_hash >= other.m_hash)
    {
        return false;
    }

    return GetClashIndex() < other.GetClashIndex();
}

size_t MStringId::GetClashIndex() const
{
    const size_t nThreadId = MThreadPool::GetCurrentThreadIndex();
    MORTY_ASSERT(nThreadId < MGlobal::M_MAX_THREAD_NUM);

    auto& clashIndex = *m_clashIndex;

    //clashIndex is idx of ClashStringTable + 1, 0 is invalid value.
    if (clashIndex[nThreadId] != 0)
    {
        return clashIndex[nThreadId];
    }

    for (size_t nIdx = 0; nIdx < ClashStringTable.size(); ++nIdx)
    {
        if (ClashStringTable[nIdx] == *m_string)
        {
            //clashIndex is idx of ClashStringTable + 1, 0 is invalid value.
            return clashIndex[nThreadId] = nIdx + 1;
        }
    }

    ClashStringTable.push_back(*m_string);
    //clashIndex is idx of ClashStringTable + 1, 0 is invalid value.
    return clashIndex[nThreadId] = ClashStringTable.size();
}