/**
 * @File         MViewport
 * 
 * @Created      2019-09-24 22:20:10
 *
 * @Author       DoubleYe
**/

#ifndef _M_MVIEWPORT_H_
#define _M_MVIEWPORT_H_
#include "MGlobal.h"
#include "MObject.h"
#include "Vector.h"
#include "Matrix.h"
#include "MBounds.h"
#include "MCameraFrustum.h"

#include <vector>

class MEntity;
class MScene;
class MPainter;
class MIRenderer;
class MInputEvent;
class MPointLight;
class MInputManager;
class MDirectionalLight;
class MORTY_API MViewport : public MObject
{
public:
	MORTY_CLASS(MViewport);

	MViewport();
	virtual ~MViewport();

public:

	void SetScene(MScene* pScene);
	MScene* GetScene(){ return m_pScene; }

	void SetCamera(MEntity* pCamera);
	MEntity* GetCamera() const;

	bool IsUseDefaultCamera() { return nullptr == m_pUserCamera; }

	void SetLeftTop(const Vector2& v2LeftTop) { m_v2LeftTop = v2LeftTop; }
	Vector2 GetLeftTop() { return m_v2LeftTop; }

	void SetSize(const Vector2& v2Size);
	Vector2 GetSize() const { return m_v2Size; }

	float GetLeft() { return m_v2LeftTop.x; }
	float GetTop() { return m_v2LeftTop.y; }
	float GetWidth() { return m_v2Size.x; }
	float GetHeight() { return m_v2Size.y; }

	MCameraFrustum& GetCameraFrustum() { return m_cameraFrustum; }


	bool ConvertWorldPointToViewport(const Vector3& v3WorldPos, Vector3& v3Result);

	void ConvertViewportPointToWorld(const Vector2& v2ViewportPos, const float& fDepth, Vector3& v3Result);

	bool ConvertWorldLineToNormalizedDevice(const Vector3& v3Pos1, const Vector3& v3Pos2, Vector2& v3Rst1, Vector2& v3Rst2);

	bool ConvertWorldPointToNormalizedDevice(const Vector3& v3Pos, Vector2& v2Rst);

	bool ConvertScreenPointToViewport(const Vector2& v2Point, Vector2& v2Result);

public:
	virtual void OnCreated() override;
	virtual void OnDelete() override;

	void LockMatrix();
	void UnlockMatrix();

	virtual void Input(MInputEvent* pEvent);

	const Matrix4& GetCameraInverseProjection() const { return m_m4CameraInvProj; }

	Matrix4 GetLightInverseProjection(MPointLight* pLight);

	
	//************************************
	// Method:    GetLightInverseProjection
	// FullName:  MViewport::GetLightInverseProjection
	// Access:    public 
	// Returns:   Matrix4
	// Qualifier:
	// Parameter: MDirectionalLight * pLight				方向光
	// Parameter: const MBoundsAABB & cMeshRenderAABB		所有响应光照的Mesh的AABB
	// Parameter: const MBoundsAABB & cShadowRenderAABB		所有产生阴影的Mesh的AABB
	//************************************
	Matrix4 GetLightInverseProjection(MEntity* pLight, const MBoundsAABB& cCaclShadowRenderAABB, const MBoundsAABB& cGenerateShadowAABB);

	void GetCameraFrustum(MEntity* pCamera, const float& fZNear, const float& fZFar, std::vector<Vector3>& vPoints);
	void GetCameraFrustum(MEntity* pCamera, const float& fZNear, const float& fZFar, Vector3& v3NearTopLeft, Vector3& v3NearTopRight, Vector3& v3NearBottomRight, Vector3& v3NearBottomLeft, Vector3& v3FarTopLeft, Vector3& v3FarTopRight, Vector3& v3FarBottomRight, Vector3& v3FarBottomLeft);
	void GetCameraFrustum(Vector3& v3NearTopLeft, Vector3& v3NearTopRight, Vector3& v3NearBottomRight, Vector3& v3NearBottomLeft, Vector3& v3FarTopLeft, Vector3& v3FarTopRight, Vector3& v3FarBottomRight, Vector3& v3FarBottomLeft);

	static Matrix4 MatrixPerspectiveFovLH(const float& fFovYZAngle, const float& fScreenAspect, const float& fScreenNear, const float& fScreenFar);
	static Matrix4 MatrixOrthoOffCenterLH(const float& fLeft, const float& fRight, const float& fTop, const float& fBottom, const float& fNear, const float& fFar);


	void SetScreenPosition(const Vector2& v2Position) { m_v2ScreenPosition = v2Position; }
	void SetScreenScale(const Vector2& v2Scale) { m_v2ScreenScale = v2Scale; }

public:

	

protected:
	void SetValidCamera(MEntity* pCamera);

	void UpdateMatrix();



private:

	MScene* m_pScene;
	
	MEntity* m_pUserCamera;

	Vector2 m_v2LeftTop;
	Vector2 m_v2Size;

	Vector2 m_v2ScreenPosition;
	Vector2 m_v2ScreenScale;

	Matrix4 m_m4CameraInvProj;
	bool m_bCameraInvProjMatrixLocked;

	MCameraFrustum m_cameraFrustum;
};


#endif
