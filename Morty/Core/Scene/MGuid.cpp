#include "Scene/MGuid.h"
#include "crossguid/guid.hpp"

using namespace morty;

MGuid MGuid::invalid = MGuid(0, 0, 0, 0);

MGuid::MGuid()
	: data()
{
}

MGuid::MGuid(const uint32_t& data0, const uint32_t& data1, const uint32_t& data2, const uint32_t& data3)
	: data({data0, data1, data2, data3})
{
}

MGuid::MGuid(const MGuid& other)
	: data(other.data)
{

}

MGuid MGuid::generate()
{
	xg::Guid guid = xg::newGuid();

	MGuid mguid;
	memcpy(mguid.data.data(), guid.bytes().data(), sizeof(uint32_t) * 4);

	return mguid;
}

const MGuid& MGuid::operator= (const MGuid& other)
{
	data = other.data;
	return other;
}

bool MGuid::operator== (const MGuid& other) const
{
	if(data[0] != other.data[0])
		return false;
	if(data[1] != other.data[1])
		return false;
	if(data[2] != other.data[2])
		return false;
	if(data[3] != other.data[3])
		return false;
	
	return true;
}

bool MGuid::operator!= (const MGuid& other) const
{
	return !operator==(other);
}

bool MGuid::operator< (const MGuid& other) const
{
	if(data[0] != other.data[0])
		return data[0] < other.data[0];
	if(data[1] != other.data[1])
		return data[1] < other.data[1];
	if(data[2] != other.data[2])
		return data[2] < other.data[2];
	if(data[3] != other.data[3])
		return data[3] < other.data[3];
		
	return false;
}
