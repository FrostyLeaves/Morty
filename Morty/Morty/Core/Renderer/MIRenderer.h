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
#include "Type/MColor.h"
#include "MRenderStructure.h"
#include "Shader/MShaderParam.h"

#include <stack>

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


	EAlphaOverlying,

	EBlendTypeEnd,
};

enum class MEDepthStencilType
{
	EDefault = 0,
	EReadNotWrite,
	ENotReadNotWrite,

	EDepthTypeEnd,
};

enum class MEMaterialType
{
	EDefault = 0,
	ETransparent,



	EBlendTransparent,

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
struct MShaderConstantParam;
class MTexture;
class MViewport;
class MIRenderTarget;
class MShaderParamSet;
class MRenderDepthTexture;
class MRenderTargetTexture;


class MORTY_CLASS MIRenderer
{
public:

	MIRenderer();;
	virtual ~MIRenderer(){};

	struct RenderTargetPair
	{
		RenderTargetPair(MIRenderTarget* pRT, MRenderDepthTexture* pDT) :pRenderTarget(pRT), pDepthTexture(pDT) {}
		MIRenderTarget* pRenderTarget;
		MRenderDepthTexture* pDepthTexture;
	};

public:

	virtual bool Initialize() = 0;
	virtual void Release() = 0;

	virtual void SetViewport(const float& fX, const float& fY, const float& fWidth, const float& fHeight, const float& fMinDepth, const float& fMaxDepth) = 0;

	virtual void Render(MIRenderTarget* pRenderTarget) = 0;

	virtual void AddOutputView(MIRenderView* pView) = 0;

public:

	virtual void DrawMesh(MIMesh* pMesh) = 0;

	virtual bool SetUseMaterial(MMaterial* pMaterial) = 0;

public:
	virtual void SetShaderParamSet(MShaderParamSet* pParamSet) = 0;

protected:

	std::stack<MIRenderTarget*> m_vRenderTargets;
};


#endif
