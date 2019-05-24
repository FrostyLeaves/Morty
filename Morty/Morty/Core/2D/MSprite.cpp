#include "MSprite.h"

MSprite::MSprite()
	: m_v2Position(0.0f, 0.0f)
	, m_v2Scale(1.0f, 1.0f)
	, m_fRotation(0.0f)
{

}

MSprite::~MSprite()
{

}

XMMATRIX MSprite::GetWorldMatrix()
{
	XMMATRIX translation = XMMatrixTranslation(m_v2Position.x, m_v2Position.y, 0.0f);
	XMMATRIX rotationZ = XMMatrixRotationZ(m_fRotation);
	XMMATRIX scale = XMMatrixScaling(m_v2Scale.x, m_v2Scale.y, 1.0f);

	return translation * rotationZ * scale;
}

void MSprite::SetPosition(const XMFLOAT2& v2Position)
{
	m_v2Position = v2Position;
}

void MSprite::SetRotation(const float& fRotation)
{
	m_fRotation = fRotation;
}

void MSprite::SetScale(const XMFLOAT2& v2Scale)
{
	m_v2Scale = v2Scale;
}
