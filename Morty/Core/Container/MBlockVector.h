/**
 * @File         MBlockVector
 * 
 * @Created      2022-02-17 10:51:49
 *
 * @Author       DoubleYe
**/

#pragma once
#include "Utility/MGlobal.h"

#include <iterator>

MORTY_SPACE_BEGIN

template<typename TYPE, size_t SIZE> class MBlockVector;

template<typename TYPE, size_t SIZE>
class MBlockVectorIter
{
public:

	explicit MBlockVectorIter(MBlockVector<TYPE, SIZE>& vec, const size_t& pid, const size_t& sid) : vArray(vec), nPrimaryIdx(pid), nSecondaryIdx(sid) {}


	TYPE& operator*();

	TYPE* operator->();

	MBlockVectorIter& operator++()
	{
		if (++nSecondaryIdx == SIZE)
		{
			nSecondaryIdx = 0;
			++nPrimaryIdx;
		}
		return *this;
	}

	MBlockVectorIter& operator--()
	{
		if (nSecondaryIdx > 0)
		{
			--nSecondaryIdx;
		}
		else
		{
			--nPrimaryIdx;
			nSecondaryIdx = SIZE - 1;
		}

		return *this;
	}

	MBlockVectorIter operator+(const size_t& len) const;
	MBlockVectorIter operator-(const size_t& len) const;

	bool operator<(const MBlockVectorIter& iter) const;
	bool operator==(const MBlockVectorIter& iter) const;
	bool operator!=(const MBlockVectorIter& iter) const;
	bool operator>(const MBlockVectorIter& iter) const;
	bool operator<=(const MBlockVectorIter& iter) const;
	bool operator>=(const MBlockVectorIter& iter) const;

	size_t id() const { return nPrimaryIdx * SIZE + nSecondaryIdx; }
	size_t pid() const { return nPrimaryIdx; }
	size_t sid() const { return nSecondaryIdx; }

protected:
	MBlockVector<TYPE, SIZE>& vArray;
	size_t nPrimaryIdx;
	size_t nSecondaryIdx;

};

template<typename TYPE, size_t SIZE>
MBlockVectorIter<TYPE, SIZE> MBlockVectorIter<TYPE, SIZE>::operator+(const size_t& len) const
{
	size_t sid = nSecondaryIdx + len;
	size_t pid = nPrimaryIdx + sid / SIZE;
	sid = sid % SIZE;

	return MBlockVectorIter<TYPE, SIZE>(vArray, pid, sid);
}

template<typename TYPE, size_t SIZE>
MBlockVectorIter<TYPE, SIZE> MBlockVectorIter<TYPE, SIZE>::operator-(const size_t& len) const
{
	if (nSecondaryIdx >= len)
	{
		size_t sid = nSecondaryIdx - len;
		return MBlockVectorIter<TYPE, SIZE>(vArray, nPrimaryIdx, sid);
	}
	else
	{
		size_t pid = nPrimaryIdx - 1 - (len - nSecondaryIdx) / SIZE;
		size_t sid = nSecondaryIdx + SIZE - len;
		return MBlockVectorIter<TYPE, SIZE>(vArray, pid, sid);
	}
}

template<typename TYPE, size_t SIZE>
bool MBlockVectorIter<TYPE, SIZE>::operator==(const MBlockVectorIter& iter) const
{
	return nPrimaryIdx == iter.nPrimaryIdx && nSecondaryIdx == iter.nSecondaryIdx;
}

template<typename TYPE, size_t SIZE>
bool MBlockVectorIter<TYPE, SIZE>::operator<(const MBlockVectorIter& iter) const
{
	if (nPrimaryIdx < iter.nPrimaryIdx) return true;
	else if (nPrimaryIdx > iter.nPrimaryIdx) return false;

	return nSecondaryIdx < iter.nSecondaryIdx;
}

template<typename TYPE, size_t SIZE>
bool MBlockVectorIter<TYPE, SIZE>::operator!=(const MBlockVectorIter& iter) const
{
	return nPrimaryIdx != iter.nPrimaryIdx || nSecondaryIdx != iter.nSecondaryIdx;
}

template<typename TYPE, size_t SIZE>
bool MBlockVectorIter<TYPE, SIZE>::operator>(const MBlockVectorIter& iter) const
{
	if (nPrimaryIdx > iter.nPrimaryIdx) return true;
	else if (nPrimaryIdx < iter.nPrimaryIdx) return false;

	return nSecondaryIdx > iter.nSecondaryIdx;
}

template<typename TYPE, size_t SIZE>
bool MBlockVectorIter<TYPE, SIZE>::operator<=(const MBlockVectorIter& iter) const
{
	if (nPrimaryIdx > iter.nPrimaryIdx) return false;

	return nSecondaryIdx <= iter.nSecondaryIdx;
}

template<typename TYPE, size_t SIZE>
bool MBlockVectorIter<TYPE, SIZE>::operator>=(const MBlockVectorIter& iter) const
{
	if (nPrimaryIdx < iter.nPrimaryIdx) return false;

	return nSecondaryIdx >= iter.nSecondaryIdx;
}

template<typename TYPE, size_t SIZE>
class MORTY_API MBlockVector
{
public:
	MBlockVector();

public:
	MBlockVectorIter<TYPE, SIZE> begin();
	MBlockVectorIter<TYPE, SIZE> end();

	TYPE& push_back(const TYPE& value);
	void pop_back();
	void clear();

	TYPE* get(const size_t& id) { return get(id / SIZE, id % SIZE); }
	TYPE* get(const size_t& pid, const size_t& sid);

public:
	static const size_t ArraySize = SIZE;
	typedef MBlockVectorIter<TYPE, SIZE> iterator;

	struct MBlockArray
	{
		std::array<TYPE, SIZE> vSecondaryArray = {};
		size_t nValidNum = 0;
	};

	std::vector<std::unique_ptr<MBlockArray>> vPrimaryArray;
};

template<typename TYPE, size_t SIZE>
MBlockVector<TYPE, SIZE>::MBlockVector()
	: vPrimaryArray()
{
	vPrimaryArray.push_back(std::make_unique<MBlockArray>());
}

template<typename TYPE, size_t SIZE>
TYPE* MBlockVector<TYPE, SIZE>::get(const size_t& pid, const size_t& sid)
{
	if (pid < vPrimaryArray.size())
	{
		auto& vec = vPrimaryArray[pid];
		if (sid < vec->nValidNum)
		{
			return &(vec->vSecondaryArray[sid]);
		}
	}

	return nullptr;
}

template<typename TYPE, size_t SIZE>
TYPE& MBlockVector<TYPE, SIZE>::push_back(const TYPE& value)
{
	if (vPrimaryArray.back()->nValidNum == SIZE)
	{
		vPrimaryArray.push_back(std::make_unique<MBlockArray>());
		std::unique_ptr<MBlockArray>& arr = vPrimaryArray.back();
		arr->vSecondaryArray[0] = value;
		arr->nValidNum = 1;
		return arr->vSecondaryArray[0];
	}
	else
	{
		std::unique_ptr<MBlockArray>& arr = vPrimaryArray.back();
		arr->vSecondaryArray[arr->nValidNum++] = value;
		return arr->vSecondaryArray[arr->nValidNum - 1];
	}
}

template<typename TYPE, size_t SIZE>
void MBlockVector<TYPE, SIZE>::pop_back()
{
	if (vPrimaryArray.back().nValidNum > 1)
	{
		std::unique_ptr<MBlockArray>& arr = vPrimaryArray.back();
		--arr->nValidNum;
	}
	else if (vPrimaryArray.back().nValidNum == 1)
	{
		if (vPrimaryArray.size() > 1)
		{
			vPrimaryArray.pop_back();
		}
		else
		{
			vPrimaryArray.back().nValidNum = 0;
		}
	}
	else
	{
		MORTY_ASSERT(false);
	}
}

template<typename TYPE, size_t SIZE>
MBlockVectorIter<TYPE, SIZE> MBlockVector<TYPE, SIZE>::begin()
{
	return MBlockVectorIter<TYPE, SIZE>(*this, 0, 0);
}

template<typename TYPE, size_t SIZE>
MBlockVectorIter<TYPE, SIZE> MBlockVector<TYPE, SIZE>::end()
{
	return MBlockVectorIter<TYPE, SIZE>(*this, vPrimaryArray.size() - 1, vPrimaryArray.back()->nValidNum);
}

template<typename TYPE, size_t SIZE>
void MBlockVector<TYPE, SIZE>::clear()
{
	vPrimaryArray.clear();
}


template<typename TYPE, size_t SIZE>
TYPE& MBlockVectorIter<TYPE, SIZE>::operator*()
{
	return vArray.vPrimaryArray[nPrimaryIdx]->vSecondaryArray[nSecondaryIdx];
}

template<typename TYPE, size_t SIZE>
TYPE* MBlockVectorIter<TYPE, SIZE>::operator->()
{
	return &(vArray.vPrimaryArray[nPrimaryIdx]->vSecondaryArray[nSecondaryIdx]);
}

MORTY_SPACE_END