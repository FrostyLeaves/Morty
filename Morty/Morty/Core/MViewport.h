/**
 * @File         MViewport
 * 
 * @Created      2019-09-24 22:20:10
 *
 * @Author       Pobrecito
**/

#ifndef _M_MVIEWPORT_H_
#define _M_MVIEWPORT_H_
#include "MGlobal.h"
#include "MObject.h"
#include "Vector.h"
#include "Matrix.h"

#include "MCameraFrustum.h"

#include <vector>

class MScene;
class MCamera;
class MPainter;
class MIRenderer;
class MInputEvent;
class MBoundsAABB;
class MInputManager;
class MPointLight;
class MDirectionalLight;
class MORTY_CLASS MViewport : public MObject
{
public:
	M_OBJECT(MViewport);

	MViewport();
	virtual ~MViewport();

public:

	void SetScene(MScene* pScene);
	MScene* GetScene(){ return m_pScene; }

	void SetCamera(MCamera* pCamera);
	MCamera* GetCamera() const;
	bool IsUseDefaultCamera() { return nullptr == m_pUserCamera; }

	void SetLeftTop(const Vector2& v2LeftTop) { m_v2LeftTop = v2LeftTop; }
	Vector2 GetLeftTop() { return m_v2LeftTop; }

	void SetSize(const Vector2& v2Size);
	Vector2 GetSize() const { return m_v2Size; }

	float GetLeft() { return m_v2LeftTop.x; }
	float GetTop() { return m_v2LeftTop.y; }
	float GetWidth() { return m_v2Size.x; }
	float GetHeight() { return m_v2Size.y; }

	MCameraFrustum* GetCameraFrustum() { return &m_cameraFrustum; }

	bool ConvertWorldPointToViewport(const Vector3& v3WorldPos, Vector3& v3Result);

	void ConvertViewportPointToWorld(const Vector2& v2ViewportPos, const float& fDepth, Vector3& v3Result);

	bool ConvertWorldLineToNormalizedDevice(const Vector3& v3Pos1, const Vector3& v3Pos2, Vector2& v3Rst1, Vector2& v3Rst2);

	bool ConvertWorldPointToNormalizedDevice(const Vector3& v3Pos, Vector2& v2Rst);

public:
	virtual void OnCreated() override;

	virtual void Render(MIRenderer* pRenderer);

	virtual void Input(MInputEvent* pEvent);

	const Matrix4& GetCameraInverseProjection() const { return m_m4CameraInvProj; }

	Matrix4 GetLightInverseProjection(MPointLight* pLight);
	Matrix4 GetLightInverseProjection(MDirectionalLight* pLight);

	void GetCameraFrustum(MCamera* pCamera, const float& fZNear, const float& fZFar, std::vector<Vector3>& vPoints);
	void GetCameraFrustum(MCamera* pCamera, const float& fZNear, const float& fZFar, Vector3& v3NearTopLeft, Vector3& v3NearTopRight, Vector3& v3NearBottomRight, Vector3& v3NearBottomLeft, Vector3& v3FarTopLeft, Vector3& v3FarTopRight, Vector3& v3FarBottomRight, Vector3& v3FarBottomLeft);
	void GetCameraFrustum(Vector3& v3NearTopLeft, Vector3& v3NearTopRight, Vector3& v3NearBottomRight, Vector3& v3NearBottomLeft, Vector3& v3FarTopLeft, Vector3& v3FarTopRight, Vector3& v3FarBottomRight, Vector3& v3FarBottomLeft);
	MBoundsAABB* GetFrustumAABB();

	static Matrix4 MatrixPerspectiveFovLH(const float& fFovYZAngle, const float& fScreenAspect, const float& fScreenNear, const float& fScreenFar);
	static Matrix4 MatrixOrthoOffCenterLH(const float& fLeft, const float& fRight, const float& fTop, const float& fBottom, const float& fNear, const float& fFar);


protected:
	void SetValidCamera(MCamera* pCamera);

	void UpdateMatrix();




private:

	MScene* m_pScene;
	
	MCamera* m_pUserCamera;
	MCamera* m_pDefaultCamera;

	Vector2 m_v2LeftTop;
	Vector2 m_v2Size;

	Matrix4 m_m4CameraInvProj;
	bool m_bCameraInvProjMatrixLocked;

	MCameraFrustum m_cameraFrustum;
};


#endif
