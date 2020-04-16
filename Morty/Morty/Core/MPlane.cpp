#include "MPlane.h"
#include "MMath.h"

MPlane::MPlane()
	: m_v4Plane(0.0f, 1.0f, 0.0f, 0.0f)
{

}

MPlane::~MPlane()
{

}

bool MPlane::IsOnFront(const Vector3& position)
{
	return (position * m_v3ABC) + m_fD > 0.0f;
}

float MPlane::GetDistance(const Vector3& position)
{
	return ((position * m_v3ABC) + m_fD) / m_v3ABC.Length();
}
