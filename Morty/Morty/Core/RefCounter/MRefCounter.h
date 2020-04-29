/**
 * @File         MRefCounter
 * 
 * @Created      2019-12-15 18:09:30
 *
 * @Author       Pobrecito
**/

#ifndef _M_MREFCOUNTER_H_
#define _M_MREFCOUNTER_H_
#include "MGlobal.h"

#include <functional>

class MORTY_CLASS MRefCounter
{
public:
	MRefCounter() : m_unReferenceNumber(0) {}
	virtual ~MRefCounter() {}

	typedef std::function<void(MRefCounter*)> MRefZeroFunction;

public:

	unsigned int GetRefNumber() { return m_unReferenceNumber; }

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
	unsigned int m_unReferenceNumber;
};


#endif
