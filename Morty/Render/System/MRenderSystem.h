/**
 * @File         MRenderSystem
 * 
 * @Created      2021-07-15 11:10:16
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MRenderGlobal.h"
#include "Basic/MCameraFrustum.h"
#include "Component/MComponent.h"
#include "Engine/MSystem.h"
#include "RHI/MRenderPass.h"

namespace morty
{

class MBuffer;
class MIDevice;
class MTaskNode;
class MViewport;
class MMeshManager;
class MSceneComponent;
class MCameraComponent;
class MORTY_API MRenderSystem : public MISystem
{
public:
    MORTY_CLASS(MRenderSystem)
public:
    MRenderSystem();

    virtual ~MRenderSystem();

public:
    void Update(MTaskNode* pNode);

public:
    MIDevice* GetDevice() const;

public:
    void Initialize() override;

    void Release() override;


public:
    void ResizeFrameBuffer(MRenderPass& renderpass, const Vector2i& v2Size);

    void ReleaseRenderpass(MRenderPass& renderpass, bool bClearTexture);

    static MCameraFrustum
    GetCameraFrustum(MViewport* pViewport, MCameraComponent* pCameraComponent, MSceneComponent* pSceneComponent);

    static Matrix4 GetCameraViewMatrix(MSceneComponent* pSceneComponent);

    static Matrix4 GetPerspectiveProjectionMatrix(
            const float fViewportWidth,
            const float fViewportHeight,
            const float fNear,
            const float fRar,
            const float fFov
    );

    static Matrix4
    GetOrthoOffProjectionMatrix(const float fWidth, const float fHeight, const float fNear, const float fRar);

    static Matrix4 GetCameraInverseProjection(
            const MViewport*        pViewport,
            const MCameraComponent* pCameraComponent,
            MSceneComponent*        pSceneComponent
    );

    static Matrix4 GetCameraProjectionMatrix(
            const MCameraComponent* pCameraComponent,
            float                   fViewWidth,
            float                   fViewHeight,
            float                   fZNear,
            float                   fZFar
    );

    static Matrix4 GetCameraInverseProjection(
            const MCameraComponent* pCameraComponent,
            MSceneComponent*        pSceneComponent,
            float                   fViewWidth,
            float                   fViewHeight,
            float                   fZNear,
            float                   fZFar
    );

public:
    static void GetCameraFrustumPoints(
            MEntity*              pCamera,
            const Vector2i&       v2ViewportSize,
            const float&          fZNear,
            const float&          fZFar,
            std::vector<Vector3>& vPoints
    );

    static void GetCameraFrustumPoints(
            MEntity*        pCamera,
            const Vector2i& v2ViewportSize,
            const float&    fZNear,
            const float&    fZFar,
            Vector3&        v3NearTopLeft,
            Vector3&        v3NearTopRight,
            Vector3&        v3NearBottomRight,
            Vector3&        v3NearBottomLeft,
            Vector3&        v3FarTopLeft,
            Vector3&        v3FarTopRight,
            Vector3&        v3FarBottomRight,
            Vector3&        v3FarBottomLeft
    );


    static Matrix4 MatrixPerspectiveFovLH(
            const float& fFov,
            const float& fScreenAspect,
            const float& fScreenNear,
            const float& fScreenFar
    );

    static Matrix4 MatrixOrthoOffCenterLH(
            const float& fLeft,
            const float& fRight,
            const float& fTop,
            const float& fBottom,
            const float& fNear,
            const float& fFar
    );

private:
    MIDevice* m_device = nullptr;
};

}// namespace morty