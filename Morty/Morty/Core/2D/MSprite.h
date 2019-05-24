#ifndef _M_SPRITE_H_
#define _M_SPRITE_H_

#include "MGlobal.h"
#include <DirectXMath.h>
using namespace DirectX;

class MORTY_CLASS MSprite
{
public:
	MSprite();
	virtual ~MSprite();


public:
	XMMATRIX GetWorldMatrix();

	void SetPosition(const XMFLOAT2& v2Position);
	void SetRotation(const float& fRotation);
	void SetScale(const XMFLOAT2& v2Scale);


private:
	XMFLOAT2 m_v2Position;
	XMFLOAT2 m_v2Scale;
	float m_fRotation;
};



#endif