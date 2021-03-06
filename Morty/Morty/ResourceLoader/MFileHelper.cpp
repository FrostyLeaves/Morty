#include "MFileHelper.h"

#include <fstream>
#include <sstream>
#include <float.h>

#ifdef MORTY_WIN
#include  <io.h>
#include <direct.h>

#endif

MMortyFileFormat::~MMortyFileFormat()
{
	for (MFormatBody& body : m_vBody)
	{
		if (!body.bExternalMemory)
		{
			delete[] body.pData;
		}
	}

	m_vBody.clear();
}

void MMortyFileFormat::PushBackBody(void* pData, const uint32_t& unSize, const bool& bExternalMemory /*= true*/)
{
	MFormatBody body;
	body.pData = (char*)pData;
	body.unSize = unSize;
	body.bExternalMemory = bExternalMemory;

	m_vBody.push_back(body);
}

const int MMortyFileFormat::s_nClipSize = 4;


MFileHelper::MFileHelper()
{

}

MFileHelper::~MFileHelper()
{

}

bool MFileHelper::MakeDir(MString strDirPath)
{
    
#ifdef MORTY_WIN
	uint32_t nChIdx = 0;
	uint32_t nCopyIdx = 0;
	char svPath[MAX_PATH] = { 0 };
	
	for (nChIdx = 0; nChIdx < strDirPath.size(); ++nChIdx)
	{
		if (strDirPath[nChIdx] == '\\' || strDirPath[nChIdx] == '/')
		{
			strncpy(svPath + nCopyIdx, strDirPath.c_str() + nCopyIdx, nChIdx - nCopyIdx);
			nCopyIdx = nChIdx;

			if (_access(svPath, 0) == -1)
			{
				if (0 != _mkdir(svPath))
					return false;
			}
		}
	}

	if (nCopyIdx != strDirPath.size())
	{
		return 0 == _mkdir(strDirPath.c_str());
	}
#endif

	return true;
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

	file.write((const char*)(&nHeadSize), nClipSize);

	file.write(&format.m_strHead[0], nHeadSize);

	for (uint32_t i = 0; i < format.m_vBody.size(); ++i)
	{
		file.write(format.m_vBody[i].pData, format.m_vBody[i].unSize);
	}
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
	file.read((char*)&nHeadSize, nClipSize);


	if (nFileLength < nClipSize + nHeadSize)
		return false;

	int nBodySize = nFileLength - nClipSize - nHeadSize;


	//SignClip : 0 ~ nClipSize
	//Head : nClipSize ~ nClipSize + nHeadSize
	//Body : nClipSize + nHeadSize ~ End

	//Warning >= C++11!!!
	format.m_strHead.resize(nHeadSize);
	file.read(&format.m_strHead[0], nHeadSize);

	char* pData = new char[nBodySize];
	file.read(pData, nBodySize);

	format.PushBackBody(pData, nBodySize, false);


	file.close();

	return true;
}

void MFileHelper::GetValidFileName(MString& strFileName)
{
	char ch[] = {
	0x5C, //    \ 
	0x2F, //    /
	0x3A, //    : 
	0x2A, //    * 
	0x3F, //    ? 
	0x22, //    " 
	0x3C, //    < 
	0x3E, //    > 
	0x7C, //    |
	};

	int nSize = strFileName.size();
	int n = 0;
	int m = 0; // \0

	while ((++n, ++m) < nSize)
	{
		for (int i = 0; i < sizeof(ch); ++i)
		{
			if (ch[i] == strFileName[n])
			{
				++m;
				break;
			}
		}
		if (n != m)
		{
			strFileName[n] = strFileName[m];
		}
	}

	strFileName.resize(n);
	
}

MString MFileHelper::GetFileFolder(const MString& strFilePath)
{
	size_t n = strFilePath.find_last_of('/');

	if (MString::npos == n)
		return "";

	return MString(strFilePath.data(), n);
}
