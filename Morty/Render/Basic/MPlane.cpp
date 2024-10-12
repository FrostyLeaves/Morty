#include "Basic/MPlane.h"
#include "Math/MMath.h"

using namespace morty;

MPlane::MPlane()
    : m_plane(0.0f, 1.0f, 0.0f, 0.0f)
{}

MPlane::~MPlane() {}

bool  MPlane::IsOnFront(const Vector3& position) const { return (position * m_plane.GetVector3()) + m_plane.w > 0.0f; }

float MPlane::GetDistance(const Vector3& position) const
{
    return ((position * m_plane.GetVector3()) + m_plane.w) / m_plane.GetVector3().Length();
}

void MPlane::MoveInNormal(const float& distance) { m_plane.w -= distance; }
