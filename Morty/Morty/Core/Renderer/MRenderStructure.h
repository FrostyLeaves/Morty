/**
 * @File         MVertex
 * 
 * @Created      2019-12-7 16:36:40
 *
 * @Author       Pobrecito
**/

#ifndef _M_MRENDERSTRUCTURE_H_
#define _M_MRENDERSTRUCTURE_H_
#include "MGlobal.h"
#include "Vector.h"
#include "MString.h"
#include "MVariant.h"
#include <map>
#include <vector>

class MIRenderer;
class MITexture;
class MTextureCube;

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
	struct ID3D11Texture2D* m_pTextureBuffer;
	class ID3D11ShaderResourceView* m_pShaderResourceView;
#elif RENDER_GRAPHICS == MORTY_OPENGLES

#endif
};

//用于渲染深度的纹理缓存
class MDepthTextureBuffer : public MTextureBuffer
{
public:
	MDepthTextureBuffer();
	virtual ~MDepthTextureBuffer() {}

#if RENDER_GRAPHICS == MORTY_DIRECTX_11
	struct ID3D11DepthStencilView* m_pDepthStencilView;
#elif RENDER_GRAPHICS == MORTY_OPENGLES

#endif
};

struct MShaderParam
{
	MShaderParam();

	MString strName;
	unsigned int unCode;
	MVariant var;
	bool bDirty;

	void SetDirty() { bDirty = true; }
	
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
	unsigned int unCode;
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
	MString strName;
	unsigned int unCode;
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