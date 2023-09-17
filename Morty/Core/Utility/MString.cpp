#include "MString.h"

#ifdef MORTY_WIN
#include <windows.h>
#endif

void MStringUtil::Replace(MString& str, const MString& source, const MString& target)
{
	std::string::size_type pos = 0;
	while ((pos = str.find(source)) != std::string::npos)
	{
		str.replace(pos, source.length(), target);
	}
}

std::vector<MString> MStringUtil::Slip(MString str, const MString& delimiter)
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
std::string MStringUtil::ConvertFromWString(const std::wstring &wstr)
{
	if (wstr.empty()) return std::string();
	int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
	std::string strTo(size_needed, 0);
	WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
	return strTo;
}

std::wstring MStringUtil::ConvertToWString(const std::string &str)
{
	if (str.empty()) return std::wstring();
	int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
	std::wstring wstrTo(size_needed, 0);
	MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
	return wstrTo;
}
#endif