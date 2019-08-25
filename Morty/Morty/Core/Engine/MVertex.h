/**
 * @File         MVertex
 * 
 * @Created      2019-08-25 15:08:54
 *
 * @Author       Morty
**/

#ifndef _M_MVERTEX_H_
#define _M_MVERTEX_H_
#include "MGlobal.h"
#include "Vector.h"

class MIRenderer;

//顶点
struct MVertex
{
public:
	Vector3 position;
	Vector3 normal;
	Vector2 texCoords;
	Vector3 tangent;
	Vector3 bitangent;
};

//顶点缓存
class MVertexBuffer
{
public:
	MVertexBuffer();
	virtual ~MVertexBuffer(){}

#if	RENDER_GRAPHICS == MORTY_DIRECTX_11
	class ID3D11Buffer* m_pVertexBuffer;
	class ID3D11Buffer* m_pIndexBuffer;
#endif
};

//顶点布局
class MVertexLayout
{
	MVertexLayout();
	virtual ~MVertexLayout(){}

	void SetUseLayout(MIRenderer* pRenderer);

};


#endif
