#include "MPointLightComponent.h"

MORTY_CLASS_IMPLEMENT(MPointLightComponent, MComponent)

MPointLightComponent::MPointLightComponent()
	: MComponent()
	, m_f3Color(1.0f, 1.0f, 1.0f)
	, m_fIntensity(1.0f)
	, m_fConstant(1.0f)
	, m_fLinear(0.022f)
	, m_fQuadratic(0.0019f)
{

}

MPointLightComponent::~MPointLightComponent()
{

}

void MPointLightComponent::WriteToStruct(MStruct& srt)
{
	Super::WriteToStruct(srt);

	M_SERIALIZER_WRITE_BEGIN;

	M_SERIALIZER_WRITE_VALUE("Color", GetColorVector);
	M_SERIALIZER_WRITE_VALUE("LightIntensity", GetLightIntensity);
	M_SERIALIZER_WRITE_VALUE("Constant", GetConstant);
	M_SERIALIZER_WRITE_VALUE("Linear", GetLinear);
	M_SERIALIZER_WRITE_VALUE("Quadratic", GetQuadratic);

	M_SERIALIZER_END;
}

void MPointLightComponent::ReadFromStruct(const MStruct& srt)
{
	Super::ReadFromStruct(srt);

	M_SERIALIZER_READ_BEGIN;

	M_SERIALIZER_READ_VALUE("Color", SetColorVector, Vector4);
	M_SERIALIZER_READ_VALUE("LightIntensity", SetLightIntensity, Float);
	M_SERIALIZER_READ_VALUE("Constant", SetConstant, Float);
	M_SERIALIZER_READ_VALUE("Linear", SetLinear, Float);
	M_SERIALIZER_READ_VALUE("Quadratic", SetQuadratic, Float);

	M_SERIALIZER_END;
}
