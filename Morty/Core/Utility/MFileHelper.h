/**
 * @File         MFileHelper
 * 
 * @Created      2019-10-24 23:44:23
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"
#include "Utility/MString.h"

#include <vector>

class MORTY_API MMortyFileFormat
{
public:
    struct MFormatBody
    {
        char* pData = nullptr;
        size_t nSize = 0;
        bool bExternalMemory = true;
    };
public:

    MMortyFileFormat() {}
    ~MMortyFileFormat();

    void PushBackBody(void* pData, const size_t& unSize, const bool& bExternalMemory = true);

    MString m_strHead;
	std::vector<MFormatBody> m_vBody;

    static const int s_nClipSize;
};

class MORTY_API MFileHelper
{
public:
    MFileHelper();
    virtual ~MFileHelper();

public:

    static bool MakeDir(MString strDirPath);

	static bool WriteString(const MString& strFilePath, const MString& strData);

	static bool ReadString(const MString& strFilePath, MString& strData);

    static bool WriteData(const MString& strFilePath, const std::vector<MByte>& vData);

    static bool ReadData(const MString& strFilePath, std::vector<MByte>& vData);

    static bool WriteFormatFile(const MString& strFilePath, const MMortyFileFormat& format);

    static bool ReadFormatFile(const MString& strFilePath, MMortyFileFormat& format);

    static void GetValidFileName(MString& strFileName);

    static MString FormatPath(MString strFilePath);

    static MString GetFileFolder(MStringView strFilePath);

    static MString GetFileName(MStringView strFullPath);

    static MString ReplaceFileName(MStringView strFullPath, MStringView strNewFileName);
    
private:

};
