/**
 * @File         MString
 * 
 * @Created      2019-05-20 00:20:18
 *
 * @Author       DoubleYe
**/

#ifndef _M_MSTRING_H_
#define _M_MSTRING_H_

#include <string>

typedef std::string MString;


class MStringHelper
{
public:
	static MString ToString(const int& value) { return std::to_string(value); }
};

#endif
