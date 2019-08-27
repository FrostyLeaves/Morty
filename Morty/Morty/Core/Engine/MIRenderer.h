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

class MIRenderView;
class MIShader;
class MNode;
class MMesh;
class MVertexBuffer;
class MShaderBuffer;
class MShader;
class MShaderResource;
class MMaterial;
class MORTY_CLASS MIRenderer
{
public:

	

	MIRenderer(){};
	virtual ~MIRenderer(){};

	virtual bool Initialize() = 0;
	virtual void Release() = 0;

	virtual void RenderNodeToView(MNode* pNode, MIRenderView* pView) = 0;

	virtual void AddOutputView(MIRenderView* pView) = 0;
	virtual void RemoveOutputView(MIRenderView* pView) = 0;
	virtual void OnResize(MIRenderView* pView, const int& nWidth, const int& nHeight) = 0;


public:
	virtual void GenerateBuffer(MVertexBuffer** ppVertexBuffer, MMesh* pMesh) = 0;
	virtual void DestroyBuffer(MVertexBuffer** ppVertexBuffer) = 0;

	virtual void CompileShader(MShaderBuffer** ppShaderBuffer, const MString& strShaderPath, const unsigned int& eShaderType) = 0;
	virtual void CleanShader(MShaderBuffer** ppShader) = 0;

	virtual void Test_DrawMesh(MMesh* pMesh) = 0;

protected:

	//TODO 应该整成 SetUseMaterialInstance
	virtual void SetUseMaterial(MMaterial* pMaterial) = 0;

};


#endif
