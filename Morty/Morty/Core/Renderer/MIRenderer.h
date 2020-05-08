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


enum class MERasterizerType
{
	EWireframe = 0,
	ECullNone,
	ECullBack,
	ECullFront,

	ERasterizerEnd
};

enum class MEBlendType
{
	EDefault = 0,
	ETransparent,

	EBlendTypeEnd,
};

enum class MEDepthStencilType
{
	EDefault = 0,
	EReadNotWrite,

	EDepthTypeEnd,
};

enum class MEMaterialType
{
	EDefault = 0,
	ETransparent,
	EDepthPeeling,

	EMaterialTypeEnd,
};


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

	virtual void SetRasterizerType(const MERasterizerType& eType) { m_eRasterizerType = eType; }

public:

	virtual bool Initialize() = 0;
	virtual void Release() = 0;

	virtual void SetViewport(const float& fX, const float& fY, const float& fWidth, const float& fHeight, const float& fMinDepth, const float& fMaxDepth) = 0;
	virtual void Render(MIRenderTarget* pRenderTarget) = 0;
	virtual void RecoverRenderTarget(MIRenderTarget* pRenderTarget) = 0;

	virtual void AddOutputView(MIRenderView* pView) = 0;

public:

	virtual void DrawMesh(MIMesh* pMesh) = 0;

	virtual bool SetUseMaterial(MMaterial* pMaterial, const bool& bUpdateResources = false) = 0;
	virtual void UpdateMaterialParam() = 0;
	virtual void UpdateMaterialResource() = 0;

public:
	virtual void SetVertexShaderParam(MShaderParam& param) = 0;
	virtual void SetPixelShaderParam(MShaderParam& param) = 0;

	virtual void SetVertexShaderTexture(MShaderTextureParam& param) = 0;
	virtual void SetPixelShaderTexture(MShaderTextureParam& param) = 0;

protected:
	MERasterizerType m_eRasterizerType;
	MEMaterialType m_eMaterialType;
};


#endif
