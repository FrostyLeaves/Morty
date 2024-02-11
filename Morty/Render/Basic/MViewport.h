/**
 * @File         MViewport
 * 
 * @Created      2019-09-24 22:20:10
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"
#include "Object/MObject.h"
#include "Math/Vector.h"
#include "Math/Matrix.h"
#include "Utility/MBounds.h"
#include "Basic/MCameraFrustum.h"

MORTY_SPACE_BEGIN

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
	MScene* GetScene() const { return m_pScene; }

	void SetCamera(MEntity* pCamera);
	MEntity* GetCamera() const;

	bool IsUseDefaultCamera() { return nullptr == m_pUserCamera; }

	void SetLeftTop(const Vector2i& n2LeftTop) { m_n2LeftTop = n2LeftTop; }
	Vector2i GetLeftTop() const { return m_n2LeftTop; }

	void SetSize(const Vector2i& n2Size);
	Vector2i GetSize() const { return m_n2Size; }

	float GetLeft() const { return m_n2LeftTop.x; }
	float GetTop() const { return m_n2LeftTop.y; }
	float GetWidth() const { return m_n2Size.x; }
	float GetHeight() const { return m_n2Size.y; }

	bool ConvertWorldPointToViewport(const Vector3& v3WorldPos, Vector3& v3Result);

	void ConvertViewportPointToWorld(const Vector2& v2ViewportPos, const float& fDepth, Vector3& v3Result);

	bool ConvertWorldLineToNormalizedDevice(const Vector3& v3Pos1, const Vector3& v3Pos2, Vector2& v3Rst1, Vector2& v3Rst2);

	bool ConvertWorldPointToNormalizedDevice(const Vector3& v3Pos, Vector2& v2Rst);

	bool ConvertScreenPointToViewport(const Vector2& v2Point, Vector2& v2Result);

public:
	virtual void OnCreated() override;
	virtual void OnDelete() override;

	virtual void Input(MInputEvent* pEvent);

	void SetScreenPosition(const Vector2i& v2Position) { m_n2ScreenPosition = v2Position; }
	void SetScreenScale(const Vector2& v2Scale) { m_f2ScreenScale = v2Scale; }

public:

	

protected:
	void SetValidCamera(MEntity* pCamera);


private:

	MScene* m_pScene;
	
	MEntity* m_pUserCamera;

	Vector2i m_n2LeftTop;
	Vector2i m_n2Size;

	Vector2i m_n2ScreenPosition;
	Vector2 m_f2ScreenScale;
};

MORTY_SPACE_END