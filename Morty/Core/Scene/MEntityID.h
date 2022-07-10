/**
 * @File         MEntity
 * 
 * @Created      2022-06-30 11:00:00
 *
 * @Author       DoubleYe
**/

#ifndef _M_MENTITY_ID_H_
#define _M_MENTITY_ID_H_
#include "crossguid/guid.hpp"

#include "MGlobal.h"

class MORTY_API MEntityID
{
public:
	MEntityID();

	static MEntityID generate();


public:

	const MEntityID& operator= (const MEntityID& other);
	bool operator== (const MEntityID& other) const ;
	bool operator< (const MEntityID& other) const;

public:
	
	static MEntityID invalid;

	xg::Guid m_id;
};


#endif