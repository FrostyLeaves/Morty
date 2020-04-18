/**
 * @File         MIRenderer
 * 
 * @Created      2019-05-12 21:49:13
 *
 * @Author       Pobrecito
**/

#ifndef _M_MIRENDERER_H_
#define _M_MIRENDERER_H_
#include "MGlobal.h"
#include "MString.h"
#include "Matrix.h"
#include "MVertex.h"
#include "MRenderStructure.h"

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
class MViewport;
class MIRenderTarget;
class MRenderTargetTexture;
class MORTY_CLASS MIRenderer
{
public:

	MIRenderer();;
	virtual ~MIRenderer(){};

public:

	enum MERasterizerType
	{
		EWireframe = 1,
		ESolid = 2,
		ECullBack = 4,
		ECullNone = 8,
		ECullFront = 16,
	};

	enum MEBlendType
	{
		ENormal = 1,
		ETransparent =2,
	};

	virtual void SetRasterizerType(const unsigned int& eType) { m_eRasterizerType = eType; }

public:

	virtual bool Initialize() = 0;
	virtual void Release() = 0;

	virtual void SetViewport(const float& fX, const float& fY, const float& fWidth, const float& fHeight, const float& fMinDepth, const float& fMaxDepth) = 0;
	virtual void Render(MIRenderTarget* pRenderTarget) = 0;
	virtual void RecoverRenderTarget(MIRenderTarget* pRenderTarget) = 0;

	virtual void AddOutputView(MIRenderView* pView) = 0;
	virtual void RemoveOutputView(MIRenderView* pView) = 0;

public:

	virtual void DrawMesh(MIMesh* pMesh) = 0;

	virtual bool SetUseMaterial(MMaterial* pMaterial) = 0;
	virtual void UpdateMaterialParam() = 0;
	virtual void UpdateMaterialResource() = 0;

public:
	virtual void SetVertexShaderParam(MShaderParam& param) = 0;
	virtual void SetPixelShaderParam(MShaderParam& param) = 0;

	virtual void SetVertexShaderTexture(MShaderTextureParam& param) = 0;
	virtual void SetPixelShaderTexture(MShaderTextureParam& param) = 0;

protected:
	unsigned int m_eRasterizerType;
	unsigned int m_eBlendType;
};


#endif
