/**
 * @File         MCameraComponent
 * 
 * @Created      2021-04-27 14:04:21
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MRenderGlobal.h"
#include "Basic/MCameraFrustum.h"
#include "Component/MComponent.h"

#include "Math/Vector.h"

namespace morty
{

class MORTY_API MCameraComponent : public MComponent
{
public:
    MORTY_CLASS(MCameraComponent)

public:
    MCameraComponent();

    virtual ~MCameraComponent();


public:
    void         SetCameraType(const MECameraType& eType) { m_cameraType = eType; }

    MECameraType GetCameraType() const { return m_cameraType; }

    void         SetFov(const float& fFov);

    float        GetFov() const { return m_fov; }

    void         SetZNear(const float& fZNear);

    float        GetZNear() const { return m_zNear; }

    void         SetZFar(const float& fZFar);

    float        GetZFar() const { return m_zFar; }

    //Orthographic
    void         SetWidth(const float& fWidth) { m_width = fWidth; }

    float        GetWidth() const { return m_width; }

    void         SetHeight(const float& fHeight) { m_height = fHeight; }

    float        GetHeight() const { return m_height; }

public:
    void SetZNearFar(const Vector2& fZNearFar)
    {
        SetZNear(fZNearFar.x);
        SetZFar(fZNearFar.y);
    }

    Vector2 GetZNearFar() const { return Vector2(GetZNear(), GetZFar()); }

public:
    virtual flatbuffers::Offset<void> Serialize(flatbuffers::FlatBufferBuilder& fbb) override;

    virtual void                      Deserialize(flatbuffers::FlatBufferBuilder& fbb) override;

    virtual void                      Deserialize(const void* pBufferPointer) override;

private:
    MECameraType m_cameraType;

    float        m_fov;
    float        m_zNear;
    float        m_zFar;

    float        m_width;
    float        m_height;
};

}// namespace morty