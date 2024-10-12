#include "MVariantMemory.h"

using namespace morty;

const size_t PackageSize = 16;

size_t       MVariantMemory::AllocMemory(size_t nAllocSize)
{
    size_t nOffset     = 0;
    size_t unRemainder = m_memorySize % PackageSize;
    if (unRemainder != 0 && (PackageSize - unRemainder) >= nAllocSize)
    {
        nOffset = m_memorySize;
        m_memorySize += nAllocSize;
    }
    else
    {
        if (unRemainder != 0)
        {
            m_memorySize = (m_memorySize / PackageSize + 1) * PackageSize;
        }

        nOffset = m_memorySize;
        m_memorySize += nAllocSize;
    }

    if (m_memory.size() < m_memorySize) { m_memory.resize(m_memorySize); }

    return nOffset;
}

void MVariantMemory::ByteAlignment()
{
    if (m_memorySize % PackageSize != 0)
    {
        m_memorySize += (PackageSize - m_memorySize % PackageSize);
    }

    if (m_memory.size() < m_memorySize) { m_memory.resize(m_memorySize); }
}

MVariantMemory& MVariantMemory::operator=(const MVariantMemory& other)
{
    m_memorySize = other.m_memorySize;
    m_memory     = other.m_memory;

    return *this;
}
