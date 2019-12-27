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
class MIViewport;
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
	};

	virtual void SetRasterizerType(const unsigned int& eType) { m_eRasterizerType = eType; }

public:

	virtual bool Initialize() = 0;
	virtual void Release() = 0;

	virtual void RenderToView(MIRenderView* pView) = 0;

	virtual void AddOutputView(MIRenderView* pView) = 0;
	virtual void RemoveOutputView(MIRenderView* pView) = 0;
	virtual void OnResize(MIRenderView* pView, const int& nWidth, const int& nHeight) = 0;


	virtual void InitDefaultResource() = 0;
	virtual void ReleaseDefaultResource() = 0;

public:

	virtual void DrawMesh(MIMesh* pMesh) = 0;

	virtual void SetUseMaterial(MMaterial* pMaterial) = 0;
	virtual void UpdateMaterialParam() = 0;
	virtual void UpdateMaterialResource() = 0;


protected:
	MTexture* m_pDefaultTexture;

	unsigned int m_eRasterizerType;
};


#endif
