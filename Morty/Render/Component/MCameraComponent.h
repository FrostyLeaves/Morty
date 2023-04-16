/**
 * @File         MCameraComponent
 * 
 * @Created      2021-04-27 14:04:21
 *
 * @Author       DoubleYe
**/

#ifndef _M_MCAMERACOMPONENT_H_
#define _M_MCAMERACOMPONENT_H_
#include "Utility/MGlobal.h"
#include "Component/MComponent.h"
#include "Basic/MCameraFrustum.h"

#include "Math/Vector.h"

class MORTY_API MCameraComponent : public MComponent
{
public:
    MORTY_CLASS(MCameraComponent)

public:
	enum class MECameraType {
		EPerspective = 1,
		EOrthographic = 2,
	};

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


#endif
