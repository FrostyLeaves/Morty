/**
 * @File         MIRenderer
 * 
 * @Created      2019-05-12 21:49:13
 *
 * @Author       Morty
**/

#ifndef _M_MIRENDERER_H_
#define _M_MIRENDERER_H_
#include "MGlobal.h"
#include "MString.h"
#include "Matrix.h"
#include "MVertex.h"

class MIRenderView;
class MIShader;
class MNode;
class MIMesh;
class MVertexBuffer;
class MShaderBuffer;
class MShader;
class MShaderResource;
class MMaterial;
class MCamera;
struct MShaderParam;
class MTexture;
class MORTY_CLASS MIRenderer
{
public:

	

	MIRenderer();;
	virtual ~MIRenderer(){};

	virtual bool Initialize() = 0;
	virtual void Release() = 0;

	virtual void RenderNodeToView(MNode* pRootNode, MCamera* pNode, MIRenderView* pView) = 0;

	virtual void AddOutputView(MIRenderView* pView) = 0;
	virtual void RemoveOutputView(MIRenderView* pView) = 0;
	virtual void OnResize(MIRenderView* pView, const int& nWidth, const int& nHeight) = 0;


	virtual void InitDefaultResource() = 0;
	virtual void ReleaseDefaultResource() = 0;

public:
	virtual void GenerateBuffer(MVertexBuffer** ppVertexBuffer, MIMesh* pMesh, const bool& bModifiable = false) = 0;
	virtual void DestroyBuffer(MVertexBuffer** ppVertexBuffer) = 0;
	virtual void UploadBuffer(MVertexBuffer** ppVertexBuffer, MIMesh* pMesh) = 0;

	virtual void GenerateTexture(MTextureBuffer** ppTextureBuffer, MTexture* pTexture, const bool& bGeneerateMipmap = true) = 0;
	virtual void DestroyTexture(MTextureBuffer** ppTextureBuffer) = 0;

	virtual void CompileShader(MShaderBuffer** ppShaderBuffer, const MString& strShaderPath, const unsigned int& eShaderType) = 0;
	virtual void CleanShader(MShaderBuffer** ppShader) = 0;

	virtual void DrawNode(MNode* pNode, const Matrix4& m4CameraInv) = 0;

	virtual void SetUseMaterial(MMaterial* pMaterial) = 0;
	virtual void UpdateShaderParam(MShaderParam& param) = 0;


protected:
	MTexture* m_pDefaultTexture;
};


#endif
