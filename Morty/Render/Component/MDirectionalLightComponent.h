/**
 * @File         MDirectionalLightComponent
 * 
 * @Created      2021-04-27 14:08:25
 *
 * @Author       Pobrecito
**/

#ifndef _M_MDIRECTIONALLIGHTCOMPONENT_H_
#define _M_MDIRECTIONALLIGHTCOMPONENT_H_
#include "MGlobal.h"
#include "MComponent.h"

#include "Vector.h"
#include "MColor.h"

class MORTY_API MDirectionalLightComponent : public MComponent
{
public:
	MORTY_CLASS(MDirectionalLightComponent)

public:
    MDirectionalLightComponent();
    virtual ~MDirectionalLightComponent();

	void SetDirection(const Vector3& v3Direction);

	Vector3 GetDirection() { return m_v3Direction; }

	Vector3 GetWorldDirection();

	void SetColor(const MColor& color) { m_f3Color = color; }
	MColor GetColor() const { return m_f3Color; }

	void SetColorVector(const Vector4& color) { m_f3Color = color; }
	Vector4 GetColorVector() const { return m_f3Color.ToVector4(); }

	// For PBR Lightning
	void SetLightIntensity(const float& fIntensity) { m_fIntensity = fIntensity; }
	float GetLightIntensity() const { return m_fIntensity; }

public:

	virtual void WriteToStruct(MStruct& srt) override;
	virtual void ReadFromStruct(const MStruct& srt) override;

private:

	Vector3 m_v3Direction;
	MColor m_f3Color;
	float m_fIntensity;

};


#endif
