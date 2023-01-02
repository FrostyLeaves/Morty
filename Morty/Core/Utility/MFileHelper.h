/**
 * @File         MFileHelper
 * 
 * @Created      2019-10-24 23:44:23
 *
 * @Author       DoubleYe
**/

#ifndef _M_MFILEHELPER_H_
#define _M_MFILEHELPER_H_
#include "Utility/MGlobal.h"
#include "Utility/MString.h"

#include <vector>

class MORTY_API MMortyFileFormat
{
public:
    struct MFormatBody
    {
        char* pData;
        uint32_t unSize;
        bool bExternalMemory;
    };
public:

    MMortyFileFormat() {}
    ~MMortyFileFormat();

    void PushBackBody(void* pData, const uint32_t& unSize, const bool& bExternalMemory = true);

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

    static MString GetFileFolder(const MString& strFilePath);
    
private:

};


#endif
