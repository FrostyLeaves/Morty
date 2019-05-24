/**
 * @File         MIShaderManager
 * 
 * @Created      2019-05-20 00:16:52
 *
 * @Author       Morty
**/

#ifndef _M_MISHADERMANAGER_H_
#define _M_MISHADERMANAGER_H_
#include "MGlobal.h"
#include "MString.h"

class MORTY_CLASS MIShaderManager
{
public:
    MIShaderManager();
    virtual ~MIShaderManager();

public:

	//加载Shader文件
	virtual MString LoadShaderFile(const MString& svFilePath) = 0;

	//移除Shader文件

private:

};


#endif
