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

class MIRenderView;
class MIShader;
class MNode;


class MMesh;
class MORTY_CLASS MIRenderer
{
public:

	class MVertexBuffer
	{

	};

	MIRenderer(){};
	virtual ~MIRenderer(){};

	virtual bool Initialize() = 0;
	virtual void Release() = 0;

	virtual void RenderNodeToView(MNode* pNode, MIRenderView* pView) = 0;

	virtual void AddOutputView(MIRenderView* pView) = 0;
	virtual void RemoveOutputView(MIRenderView* pView) = 0;
	virtual void OnResize(MIRenderView* pView, const int& nWidth, const int& nHeight) = 0;

protected:
	virtual MVertexBuffer* CreateVertexBuffer(MMesh* pMesh) = 0;
};


#endif
