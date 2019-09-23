/**
 * @File         MIDevice
 * 
 * @Created      2019-09-21 23:08:49
 *
 * @Author       Morty
**/

#ifndef _M_MIDEVICE_H_
#define _M_MIDEVICE_H_
#include "MGlobal.h"
#include "MString.h"

class MVertexBuffer;
class MIMesh;
class MTextureBuffer;
class MTexture;
class MShaderBuffer;
class MORTY_CLASS MIDevice
{
public:
	MIDevice() {}
	virtual ~MIDevice() {}

public:
	virtual bool Initialize() = 0;
	virtual void Release() = 0;

public:
	virtual void GenerateBuffer(MVertexBuffer** ppVertexBuffer, MIMesh* pMesh, const bool& bModifiable = false) = 0;
	virtual void DestroyBuffer(MVertexBuffer** ppVertexBuffer) = 0;
	virtual void UploadBuffer(MVertexBuffer** ppVertexBuffer, MIMesh* pMesh) = 0;

	virtual void GenerateTexture(MTextureBuffer** ppTextureBuffer, MTexture* pTexture, const bool& bGeneerateMipmap = true) = 0;
	virtual void GenerateTextureCube(MTextureBuffer** ppTextureBuffer, MTexture* vTexture[6], const bool& bGenerateMipmap = true) = 0;
	virtual void DestroyTexture(MTextureBuffer** ppTextureBuffer) = 0;

	virtual void CompileShader(MShaderBuffer** ppShaderBuffer, const MString& strShaderPath, const unsigned int& eShaderType) = 0;
	virtual void CleanShader(MShaderBuffer** ppShader) = 0;

};

#endif
