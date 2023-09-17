/**
 * @File         MString
 * 
 * @Created      2019-05-20 00:20:18
 *
 * @Author       DoubleYe
**/

#pragma once

#include <string>
#include <vector>

typedef std::string MString;
typedef std::string_view MStringView;

typedef std::string MPath;

class MStringUtil
{
public:
	static MString ToString(const int& value) { return std::to_string(value); }

	static void Replace(MString& str, const MString& source, const MString& target);

	static std::vector<MString> Slip(MString str, const MString& delimiter);

#ifdef MORTY_WIN
	static std::string ConvertFromWString(const std::wstring &wstr);
	static std::wstring ConvertToWString(const std::string &str);
#endif
};
