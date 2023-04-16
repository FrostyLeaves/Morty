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
#include <vector>

#ifdef MORTY_WIN
#include <windows.h>
#endif

typedef std::string MString;
typedef std::string_view MStringView;

class MStringHelper
{
public:
	static MString ToString(const int& value) { return std::to_string(value); }

	static void Replace(MString& str, const MString& source, const MString& target)
	{
		std::string::size_type pos = 0;
		while ((pos = str.find(source)) != std::string::npos)
		{
			str.replace(pos, source.length(), target);
		}
	}

	static std::vector<MString> Slip(MString str, const MString& delimiter)
	{
		std::vector<MString> vResult;
		size_t pos = 0;
		std::string token;
		while ((pos = str.find(delimiter)) != std::string::npos) {
			vResult.push_back(str.substr(0, pos));
			str.erase(0, pos + delimiter.length());
		}
		if(!str.empty())
			vResult.push_back(str);

		return vResult;
	}

#ifdef MORTY_WIN
	static void ConvertToWString(const std::string& instr, std::wstring& outstr)
	{
		// Assumes std::string is encoded in the current Windows ANSI codepage
		int bufferlen = ::MultiByteToWideChar(CP_ACP, 0, instr.c_str(), static_cast<int>(instr.size()), NULL, 0);

		if (bufferlen == 0)
		{
			// Something went wrong. Perhaps, check GetLastError() and log.
			return;
		}

		// Allocate new LPWSTR - must deallocate it later
		outstr.resize(bufferlen);

		::MultiByteToWideChar(CP_ACP, 0, instr.c_str(), static_cast<int>(instr.size()), (LPWSTR)outstr.data(), bufferlen);

		// Ensure wide string is null terminated
		outstr[bufferlen] = '\0';
	}
#endif
};

#endif
