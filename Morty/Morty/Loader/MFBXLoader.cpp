#include "MFBXLoader.h"
#include "fbxsdk.h"

#include "MLogManager.h"

using namespace FBXSDK_NAMESPACE;

fbxsdk::FbxManager* MFBXLoader::m_pFBXSDKManager = nullptr;
MFBXLoader::FBXSDKVersion MFBXLoader::m_cFBXSDKVersion;

MFBXLoader::MFBXLoader()
{

}

MFBXLoader::~MFBXLoader()
{

}

bool MFBXLoader::InitializeFBXSDK()
{
	m_pFBXSDKManager = FbxManager::Create();

	FbxIOSettings* pFbxIOSettings = FbxIOSettings::Create(m_pFBXSDKManager, IOSROOT);

	m_pFBXSDKManager->SetIOSettings(pFbxIOSettings);

	//获取SDK版本号
	FbxManager::GetFileFormatVersion(m_cFBXSDKVersion.nMajor, m_cFBXSDKVersion.nMinor, m_cFBXSDKVersion.nRevision);

	return true;
}

bool MFBXLoader::Initialize()
{
	if (nullptr == nullptr && !InitializeFBXSDK())
		return false;

	return true;
}

void MFBXLoader::Release()
{

}

bool MFBXLoader::ImportFbxModel(fbxsdk::FbxScene* pScene, const char* svImportFilePath)
{
	int nAnimStackCount;

	char svPassword[1024];

	
	FbxImporter* pImporter = FbxImporter::Create(m_pFBXSDKManager, svImportFilePath);

	const bool bImportret = pImporter->Initialize(svImportFilePath, -1, m_pFBXSDKManager->GetIOSettings());


	if (!bImportret)
	{
		//int nFileVerMajorNum, nFileVerMinorNum, nFileVerRevision;
		//pImporter->GetFileVersion(nFileVerMajorNum, nFileVerMinorNum, nFileVerRevision);

		FbxString strError = pImporter->GetStatus().GetErrorString();
		MLogManager::GetInstance()->Error("FBX Importer初始化失败: %s", strError.Buffer());

		return false;
	}

	bool bResult = pImporter->Import(pScene);

	
	pImporter->Destroy();

	return bResult;
}
