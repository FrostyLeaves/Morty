/**
 * @File         MCameraComponent
 * 
 * @Created      2021-04-27 14:04:21
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Render/MRenderGlobal.h"
#include "Component/MComponent.h"
#include "Basic/MCameraFrustum.h"

#include "Math/Vector.h"

MORTY_SPACE_BEGIN

class MORTY_API MCameraComponent : public MComponent
{
public:
    MORTY_CLASS(MCameraComponent)
		
public:
    MCameraComponent();
    virtual ~MCameraComponent();


public:

	void SetCameraType(const MECameraType& eType) { m_eCameraType = eType; }
	MECameraType GetCameraType() const { return m_eCameraType; }

	void SetFov(const float& fFov);
	float GetFov() const { return m_fFov; }

	void SetZNear(const float& fZNear);
	float GetZNear() const { return m_fZNear; }

	void SetZFar(const float& fZFar);
	float GetZFar() const { return m_fZFar; }

	//Orthographic
	void SetWidth(const float& fWidth) { m_fWidth = fWidth; }
	float GetWidth() const { return m_fWidth; }

	void SetHeight(const float& fHeight) { m_fHeight = fHeight; }
	float GetHeight() const { return m_fHeight; }

public:
	void SetZNearFar(const Vector2& fZNearFar) { SetZNear(fZNearFar.x); SetZFar(fZNearFar.y); }
	Vector2 GetZNearFar() const { return Vector2(GetZNear(), GetZFar()); }

public:

	virtual flatbuffers::Offset<void> Serialize(flatbuffers::FlatBufferBuilder& fbb) override;
	virtual void Deserialize(flatbuffers::FlatBufferBuilder& fbb) override;
	virtual void Deserialize(const void* pBufferPointer) override;

private:

	MECameraType m_eCameraType;

	float m_fFov;
	float m_fZNear;
	float m_fZFar;

	float m_fWidth;
	float m_fHeight;
};

MORTY_SPACE_END