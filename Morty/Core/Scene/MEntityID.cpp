#include "MEntityID.h"

MEntityID MEntityID::invalid = MEntityID();

MEntityID::MEntityID()
	: m_id()
{
}

MEntityID MEntityID::generate()
{
	return MEntityID();
}

const MEntityID& MEntityID::operator= (const MEntityID& other)
{
	m_id = other.m_id;
	return other;
}

bool MEntityID::operator== (const MEntityID& other) const
{
	return m_id == other.m_id;
}

bool MEntityID::operator< (const MEntityID& other) const
{
	return m_id < other.m_id;
}
