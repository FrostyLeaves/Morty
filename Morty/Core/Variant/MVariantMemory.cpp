#include "MVariantMemory.h"

using namespace morty;

const size_t PackageSize = 16;

size_t MVariantMemory::AllocMemory(size_t nAllocSize)
{
	size_t nOffset = 0;
	size_t unRemainder = m_nMemorySize % PackageSize;
	if (unRemainder != 0 && (PackageSize - unRemainder) >= nAllocSize)
	{
		nOffset = m_nMemorySize;
		m_nMemorySize += nAllocSize;
	}
	else
	{
		if (unRemainder != 0)
		{
			m_nMemorySize = (m_nMemorySize / PackageSize + 1) * PackageSize;
		}

		nOffset = m_nMemorySize;
		m_nMemorySize += nAllocSize;
	}

	if (m_vMemory.size() < m_nMemorySize)
	{
		m_vMemory.resize(m_nMemorySize);
	}

	return nOffset;
}

void MVariantMemory::ByteAlignment()
{
	if (m_nMemorySize % PackageSize != 0)
	{
		m_nMemorySize += (PackageSize - m_nMemorySize % PackageSize);
	}

	if (m_vMemory.size() < m_nMemorySize)
	{
		m_vMemory.resize(m_nMemorySize);
	}
}

MVariantMemory& MVariantMemory::operator=(const MVariantMemory& other)
{
	m_nMemorySize = other.m_nMemorySize;
	m_vMemory = other.m_vMemory;

	return *this;
}
