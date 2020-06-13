/**
 * @File         MFileHelper
 * 
 * @Created      2019-10-24 23:44:23
 *
 * @Author       Pobrecito
**/

#ifndef _M_MFILEHELPER_H_
#define _M_MFILEHELPER_H_
#include "MGlobal.h"
#include "MString.h"

class MORTY_CLASS MMortyFileFormat
{
public:

    MString m_strHead;
    MString m_strBody;

    static const int s_nClipSize;
};

class MORTY_CLASS MFileHelper
{
public:
    MFileHelper();
    virtual ~MFileHelper();

public:

	static bool WriteString(const MString& strFilePath, const MString& strData);

	static bool ReadString(const MString& strFilePath, MString& strData);

    static bool WriteFormatFile(const MString& strFilePath, const MMortyFileFormat& format);

    static bool ReadFormatFile(const MString& strFilePath, MMortyFileFormat& format);

private:

};


#endif
