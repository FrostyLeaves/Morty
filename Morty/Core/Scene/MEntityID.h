/**
 * @File         MGuid
 * 
 * @Created      2022-06-30 11:00:00
 *
 * @Author       DoubleYe
**/

#ifndef _M_GUID_H_
#define _M_GUID_H_
#include "crossguid/guid.hpp"

#include "Utility/MGlobal.h"

class MORTY_API MGuid
{
public:
	MGuid();

	static MGuid generate();


public:

	const MGuid& operator= (const MGuid& other);
	bool operator== (const MGuid& other) const ;
	bool operator< (const MGuid& other) const;

public:
	
	static MGuid invalid;

	xg::Guid m_id;
};


#endif