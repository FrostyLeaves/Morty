#include "MFileHelper.h"

#include <fstream>
#include <sstream>

const int MMortyFileFormat::s_nClipSize = 4;


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

bool MFileHelper::WriteFormatFile(const MString& strFilePath, const MMortyFileFormat& format)
{
	std::ofstream file(strFilePath.c_str(), std::ios::ate | std::ios::binary);

	if (!file.is_open())
		return false;

	const int nClipSize = MMortyFileFormat::s_nClipSize;

	const int nHeadSize = format.m_strHead.size();

	const int nBodySize = format.m_strBody.size();

	file.write((const char*)(&nHeadSize), nClipSize);

	file.write(&format.m_strHead[0], nHeadSize);

	file.write(&format.m_strBody[0], nBodySize);

	file.close();

	return true;
}

bool MFileHelper::ReadFormatFile(const MString& strFilePath, MMortyFileFormat& format)
{
	std::ifstream file(strFilePath.c_str(), std::ios::binary);

	if (!file.is_open())
		return false;

	file.seekg(0, std::ios::end);
	int nFileLength = file.tellg();
	file.seekg(0, std::ios::beg);

	const int nClipSize = MMortyFileFormat::s_nClipSize;

	if (nFileLength < nClipSize)
		return false;


	int nHeadSize = 0;
	file.read((char*)nHeadSize, nClipSize);


	if (nFileLength < nClipSize + nHeadSize)
		return false;

	int nBodySize = nFileLength - nClipSize - nHeadSize;


	//SignClip : 0 ~ nClipSize
	//Head : nClipSize ~ nClipSize + nHeadSize
	//Body : nClipSize + nHeadSize ~ End

	//Warning >= C++11!!!
	format.m_strHead.resize(nHeadSize);
	file.read(&format.m_strHead[0], nHeadSize);

	format.m_strBody.resize(nBodySize);
	file.read(&format.m_strBody[0], nBodySize);


	file.close();

	return true;
}
