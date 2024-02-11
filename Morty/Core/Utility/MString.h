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

namespace morty {

	using MString = std::string;
	using MStringView = std::string_view;
	using MPath = std::string;

	class MStringUtil
	{
	public:
		template<typename TYPE>
		static MString ToString(const TYPE& value) { return std::to_string(value); }

		static void Replace(MString& str, const MString& source, const MString& target);

		static std::vector<MString> Slip(MString str, const MString& delimiter);

#ifdef MORTY_WIN
		static std::string ConvertFromWString(const std::wstring& wstr);
		static std::wstring ConvertToWString(const std::string& str);
#endif
	};

}