/**
 * @File         MViewport
 * 
 * @Created      2019-09-24 22:20:10
 *
 * @Author       Morty
**/

#ifndef _M_MIVIEWPORT_H_
#define _M_MIVIEWPORT_H_
#include "MGlobal.h"
#include "MObject.h"
#include "Vector.h"
#include "Matrix.h"

class MIScene;
class MCamera;
class MIRenderer;
class MORTY_CLASS MIViewport : public MObject
{
public:
	MIViewport();
	virtual ~MIViewport();

public:

	void SetScene(MIScene* pScene);
	MIScene* GetScene(){ return m_pScene; }

	void SetCamera(MCamera* pCamera);
	MCamera* GetCamera();

	void SetLeftTop(const Vector2& v2LeftTop) { m_v2LeftTop = v2LeftTop; }
	Vector2 GetLeftTop() { return m_v2LeftTop; }
	void SetSize(const Vector2& v2Size) { m_v2Size = v2Size; }
	Vector2 GetSize(){ return m_v2Size; }
	float GetLeft() { return m_v2LeftTop.x; }
	float GetTop() { return m_v2LeftTop.y; }
	float GetWidth() { return m_v2Size.x; }
	float GetHeight() { return m_v2Size.y; }


public:
	virtual void OnCreated() override;

	virtual void Render(MIRenderer* pRenderer);

	Matrix4 GetCameraInverseProjection(){ return m_m4CameraInvProj; }

protected:
	void SetValidCamera(MCamera* pCamera);


	static Matrix4 MatrixPerspectiveFovLH(const float& fFovYZAngle, const float& fScreenAspect, const float& fScreenNear, const float& fScreenFar);
	

private:

	MIScene* m_pScene;
	
	MCamera* m_pUserCamera;
	MCamera* m_pDefaultCamera;

	Vector2 m_v2LeftTop;
	Vector2 m_v2Size;

	Matrix4 m_m4CameraInvProj;
};


#endif
