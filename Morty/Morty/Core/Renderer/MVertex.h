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
#include "MString.h"
#include "MVariant.h"
#include <map>
#include <vector>

class MIRenderer;
class MITexture;
class MTextureCube;
//顶点
struct MVertex
{
public:
	MVertex();

public:
	Vector3 position;
	Vector3 normal;
	Vector2 texCoords;
	Vector3 tangent;
	Vector3 bitangent;
};

class MInputLayout
{
public:
	MInputLayout();
	virtual ~MInputLayout(){}
#if RENDER_GRAPHICS == MORTY_DIRECTX_11
	class ID3D11InputLayout* m_pInputLayout;
#elif RENDER_GRAPHICS == MORTY_OPENGLES

#endif
};

//顶点缓存
class MVertexBuffer
{
public:
	MVertexBuffer();
	virtual ~MVertexBuffer(){}

#if RENDER_GRAPHICS == MORTY_DIRECTX_11
	class ID3D11Buffer* m_pVertexBuffer;
	class ID3D11Buffer* m_pIndexBuffer;
#elif RENDER_GRAPHICS == MORTY_OPENGLES

#endif
};

//纹理缓存
class MTextureBuffer
{
public:
	MTextureBuffer();
	virtual ~MTextureBuffer();

#if RENDER_GRAPHICS == MORTY_DIRECTX_11
	class ID3D11ShaderResourceView* m_pShaderResourceView;
#elif RENDER_GRAPHICS == MORTY_OPENGLES

#endif
};

struct MShaderParam
{
	MString strName;
	Variant var;
	
#if RENDER_GRAPHICS == MORTY_DIRECTX_11
	class ID3D11Buffer* pBuffer;
	unsigned int unBindPoint;
	unsigned int unBindCount;
#elif RENDER_GRAPHICS == MORTY_OPENGLES

#endif
};

enum METextureType
{
	ETexture2D = 1,
	ETextureCube = 2,
};

struct MShaderTextureParam
{
	MString strName;
	MITexture* pTexture;
	METextureType eType;

#if RENDER_GRAPHICS == MORTY_DIRECTX_11
	unsigned int unBindPoint;
	unsigned int unBindCount;
#elif RENDER_GRAPHICS == MORTY_OPENGLES

#endif
};

struct MShaderSampleParam
{
#if RENDER_GRAPHICS == MORTY_DIRECTX_11
	unsigned int unBindPoint;
	unsigned int unBindCount;
#elif RENDER_GRAPHICS == MORTY_OPENGLES

#endif
};

//Shader
class MShaderBuffer
{
public:
	MShaderBuffer();
	virtual ~MShaderBuffer(){}

	std::vector<MShaderSampleParam> m_vSampleParamsTemplate;
	std::vector<MShaderTextureParam> m_vTextureParamsTemplate;
	std::vector<MShaderParam> m_vShaderParamsTemplate;
};

class MVertexShaderBuffer : public MShaderBuffer
{
public:
	MVertexShaderBuffer();
	virtual ~MVertexShaderBuffer(){}
#if RENDER_GRAPHICS == MORTY_DIRECTX_11
	class ID3D11VertexShader* m_pVertexShader;
	class ID3D11InputLayout* m_pInputLayout;
#elif RENDER_GRAPHICS == MORTY_OPENGLES
#endif
};

class MPixelShaderBuffer : public MShaderBuffer
{
public:
	MPixelShaderBuffer();
	virtual ~MPixelShaderBuffer(){}
#if RENDER_GRAPHICS == MORTY_DIRECTX_11
	class ID3D11PixelShader* m_pPixelShader;
#elif RENDER_GRAPHICS == MORTY_OPENGLES
#endif
};


#endif