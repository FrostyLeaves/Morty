/**
 * @File         MFBXLoader
 * 
 * @Created      2019-07-31 19:52:42
 *
 * @Author       Morty
**/

#ifndef _M_MFBXLOADER_H_
#define _M_MFBXLOADER_H_
#include "MGlobal.h"
//#include "fbxsdk.h"
 
namespace fbxsdk{
	class FbxManager;
	class FbxScene;
};


class MORTY_CLASS MFBXLoader
{
public:
    MFBXLoader();
    virtual ~MFBXLoader();


	bool InitializeFBXSDK();

	virtual bool Initialize();
	virtual void Release();

public:

	struct FBXSDKVersion
	{
		int nMajor;
		int nMinor;
		int nRevision;
	};

protected:
	bool ImportFbxModel(fbxsdk::FbxScene* pScene, const char* svImportFilePath);

private:

	static fbxsdk::FbxManager* m_pFBXSDKManager;
	static FBXSDKVersion m_cFBXSDKVersion;
};


#endif
