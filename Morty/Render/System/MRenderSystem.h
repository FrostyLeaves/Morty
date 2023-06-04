/**
 * @File         MRenderSystem
 * 
 * @Created      2021-07-15 11:10:16
 *
 * @Author       DoubleYe
**/

#ifndef _M_MRENDERSYSTEM_H_
#define _M_MRENDERSYSTEM_H_
#include "Render/MRenderGlobal.h"
#include "Engine/MSystem.h"
#include "Component/MComponent.h"
#include "Render/MRenderPass.h"
#include "Basic/MCameraFrustum.h"

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

    virtual void Initialize();
    virtual void Release();


public:

    void ResizeFrameBuffer(MRenderPass& renderpass, const Vector2& v2Size);
    void ReleaseRenderpass(MRenderPass& renderpass, bool bClearTexture);

    static MCameraFrustum GetCameraFrustum(MViewport* pViewport, MCameraComponent* pCameraComponent, MSceneComponent* pSceneComponent);

    static Matrix4 GetCameraInverseProjection(const MViewport* pViewport, const MCameraComponent* pCameraComponent, MSceneComponent* pSceneComponent);
    static Matrix4 GetCameraInverseProjection(const MViewport* pViewport, const MCameraComponent* pCameraComponent, MSceneComponent* pSceneComponent, float fZNear, float fZFar);
    
public:

    static void GetCameraFrustumPoints(MEntity* pCamera
        , const Vector2& v2ViewportSize
        , const float& fZNear, const float& fZFar
        , std::vector<Vector3>& vPoints
    );

    static void GetCameraFrustumPoints(MEntity* pCamera
        , const Vector2& v2ViewportSize
        , const float& fZNear, const float& fZFar
        , Vector3& v3NearTopLeft, Vector3& v3NearTopRight
        , Vector3& v3NearBottomRight, Vector3& v3NearBottomLeft
        , Vector3& v3FarTopLeft, Vector3& v3FarTopRight
        , Vector3& v3FarBottomRight, Vector3& v3FarBottomLeft
    );


    static Matrix4 MatrixPerspectiveFovLH(const float& fFovYZAngle, const float& fScreenAspect, const float& fScreenNear, const float& fScreenFar);
    static Matrix4 MatrixOrthoOffCenterLH(const float& fLeft, const float& fRight, const float& fTop, const float& fBottom, const float& fNear, const float& fFar);

private:

	MIDevice* m_pDevice = nullptr;
};


#endif
