/**
 * @File         MViewport
 * 
 * @Created      2019-09-24 22:20:10
 *
 * @Author       DoubleYe
**/

#ifndef _M_MVIEWPORT_H_
#define _M_MVIEWPORT_H_
#include "Utility/MGlobal.h"
#include "Object/MObject.h"
#include "Math/Vector.h"
#include "Math/Matrix.h"
#include "Utility/MBounds.h"
#include "Basic/MCameraFrustum.h"

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

	const Matrix4& GetProjection() const { return m_m4Projection; }

	const Matrix4& GetCameraInverseProjection() const { return m_m4CameraInvProj; }

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

	Matrix4 m_m4Projection;
	Matrix4 m_m4CameraInvProj;
	bool m_bCameraInvProjMatrixLocked;

	MCameraFrustum m_cameraFrustum;
};


#endif
