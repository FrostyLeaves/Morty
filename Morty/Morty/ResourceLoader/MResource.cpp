#include "MResource.h"

MResource::MResource()
: m_unResourceID(0)
, m_pResourceManager(nullptr)
{
    
}

MResource::~MResource()
{

}


MString MResource::GetSuffix(const MString& strPath)
{
	MString suffix = strPath.substr(strPath.find_last_of('.') + 1, strPath.size());
	for (char& c : suffix)
	{
		if ('A' <= c && c <= 'Z')
			c += ('a' - 'A');
	}

	return suffix;
}
