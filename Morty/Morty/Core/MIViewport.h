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

	void SetSize(const Vector2& v2Size);
	Vector2 GetSize(){ return m_v2Size; }


public:
	virtual void OnCreated() override;

	virtual void Render(MIRenderer* pRenderer);


	Matrix4 GetCameraInverseProjection(){ return m_m4CameraInvProj; }

protected:

	void SetValidCamera(MCamera* pCamera);

private:

	MIScene* m_pScene;
	
	MCamera* m_pUserCamera;
	MCamera* m_pDefaultCamera;

	Vector2 m_v2Size;

	Matrix4 m_m4CameraInvProj;
};


#endif
