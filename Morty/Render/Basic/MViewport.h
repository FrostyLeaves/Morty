/**
 * @File         MViewport
 * 
 * @Created      2019-09-24 22:20:10
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"
#include "Basic/MCameraFrustum.h"
#include "Math/Matrix.h"
#include "Math/Vector.h"
#include "Object/MObject.h"
#include "Utility/MBounds.h"
#include "Utility/MRect.h"

namespace morty
{

class MEntity;
class MScene;
class MPainter;
class MInputEvent;
class MPointLight;
class MInputManager;
class MDirectionalLight;
class MORTY_API MViewport : public MObject
{
public:
    MORTY_CLASS(MViewport);

    MViewport();

    ~MViewport() override;

public:
    void                   SetScene(MScene* scene);

    [[nodiscard]] MScene*  GetScene() const { return m_scene; }

    void                   SetCamera(MEntity* pCamera);

    [[nodiscard]] MEntity* GetCamera() const;

    bool                   IsUseDefaultCamera() { return nullptr == m_userCamera; }

    void                   SetLeftTop(const Vector2i& n2LeftTop) { m_rect.SetPosition(n2LeftTop); }
    void                   SetSize(const Vector2i& size);
    void                   SetRect(const MRecti& rect) { m_rect = rect; }
    [[nodiscard]] Vector2i GetLeftTop() const { return m_rect.GetPosition(); }
    [[nodiscard]] Vector2i GetSize() const { return m_rect.GetSize(); }
    [[nodiscard]] float    GetLeft() const { return m_rect.GetLeft(); }
    [[nodiscard]] float    GetTop() const { return m_rect.GetTop(); }
    [[nodiscard]] float    GetWidth() const { return m_rect.GetWidth(); }
    [[nodiscard]] float    GetHeight() const { return m_rect.GetHeight(); }
    [[nodiscard]] MRecti   GetRect() const { return m_rect; }


    bool                   ConvertWorldPointToViewport(const Vector3& v3WorldPos, Vector3& v3Result);

    void ConvertViewportPointToWorld(const Vector2& v2ViewportPos, const float& fDepth, Vector3& v3Result);

    bool
    ConvertWorldLineToNormalizedDevice(const Vector3& v3Pos1, const Vector3& v3Pos2, Vector2& v3Rst1, Vector2& v3Rst2);

    bool ConvertWorldPointToNormalizedDevice(const Vector3& v3Pos, Vector2& v2Rst);

    bool ConvertScreenPointToViewport(const Vector2& v2Point, Vector2& v2Result);

public:
    void         OnCreated() override;

    void         OnDelete() override;

    virtual void Input(MInputEvent* pEvent);

    void         SetScreenPosition(const Vector2i& v2Position) { m_screenPosition = v2Position; }

    void         SetScreenScale(const Vector2& v2Scale) { m_screenScale = v2Scale; }

public:
protected:
    void SetValidCamera(MEntity* pCamera);


private:
    MScene*  m_scene;

    MEntity* m_userCamera;

    MRecti   m_rect;

    Vector2i m_screenPosition;
    Vector2  m_screenScale;
};

}// namespace morty