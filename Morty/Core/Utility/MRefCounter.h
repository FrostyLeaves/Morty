/**
 * @File         MRefCounter
 * 
 * @Created      2019-12-15 18:09:30
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"

MORTY_SPACE_BEGIN

class MORTY_API MRefCounter
{
public:
	MRefCounter() : m_unReferenceNumber(0) {}
	virtual ~MRefCounter() {}

	typedef std::function<void(MRefCounter*)> MRefZeroFunction;

public:

	uint32_t GetRefNumber() { return m_unReferenceNumber; }

	void AddRef() { ++m_unReferenceNumber; }

	void SubRef()
	{
		--m_unReferenceNumber;
		if (0 == m_unReferenceNumber)
			OnReferenceZero();
	}

protected:
	virtual void OnReferenceZero();

private:
	uint32_t m_unReferenceNumber;
};

MORTY_SPACE_END