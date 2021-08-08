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

#ifdef MORTY_WIN
#include <windows.h>
#endif

typedef std::string MString;


class MStringHelper
{
public:
	static MString ToString(const int& value) { return std::to_string(value); }

#ifdef MORTY_WIN
	static void ConvertToWString(const std::string& instr, std::wstring& outstr)
	{
		// Assumes std::string is encoded in the current Windows ANSI codepage
		int bufferlen = ::MultiByteToWideChar(CP_ACP, 0, instr.c_str(), instr.size(), NULL, 0);

		if (bufferlen == 0)
		{
			// Something went wrong. Perhaps, check GetLastError() and log.
			return;
		}

		// Allocate new LPWSTR - must deallocate it later
		outstr.resize(bufferlen);

		::MultiByteToWideChar(CP_ACP, 0, instr.c_str(), instr.size(), (LPWSTR)outstr.data(), bufferlen);

		// Ensure wide string is null terminated
		outstr[bufferlen] = '\0';
	}
#endif
};

#endif
