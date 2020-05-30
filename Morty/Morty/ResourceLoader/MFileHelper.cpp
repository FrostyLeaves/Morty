#include "MFileHelper.h"

#include <fstream>
#include <sstream>

MFileHelper::MFileHelper()
{

}

MFileHelper::~MFileHelper()
{

}

bool MFileHelper::WriteString(const MString& strFilePath, const MString& strData)
{
	std::ofstream file(strFilePath.c_str(), std::ios::ate);

	if (!file.is_open())
		return false;

	file << strData;

	file.close();

	return true;
}

bool MFileHelper::ReadString(const MString& strFilePath, MString& strData)
{
	std::ifstream file(strFilePath.c_str());

	if (!file.is_open())
		return false;

	strData.clear();
	std::stringstream buffer;
	buffer << file.rdbuf();
	strData = buffer.str();

	file.close();

	return true;
}
