/**
 * @File         MGuid
 * 
 * @Created      2022-06-30 11:00:00
 *
 * @Author       DoubleYe
**/

#pragma once

#include "MGuid_generated.h"
#include "../../Render/Flatbuffer/MBone_generated.h"
#include "Utility/MGlobal.h"

class MORTY_API MGuid
{
public:
	MGuid();
	MGuid(const uint32_t& data0, const uint32_t& data1, const uint32_t& data2, const uint32_t& data3);
	MGuid(const MGuid& other);
	static MGuid generate();


public:

	const MGuid& operator= (const MGuid& other);
	bool operator== (const MGuid& other) const ;
	bool operator!= (const MGuid& other) const;
	bool operator< (const MGuid& other) const;

public:
	
	static MGuid invalid;

	std::array<uint32_t, 4> data;
};
