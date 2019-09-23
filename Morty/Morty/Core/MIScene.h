/**
 * @File         MIScene
 * 
 * @Created      2019-09-19 00:32:56
 *
 * @Author       Morty
**/

#ifndef _M_MISCENE_H_
#define _M_MISCENE_H_
#include "MGlobal.h"
#include "MObject.h"
#include "Matrix.h"

#include <vector>

class MNode;
class MCamera;
class MSkyBox;
class MDirectionalLight;
class MPointLight;
class MIRenderer;
class MIRenderView;
class MORTY_CLASS MIScene : public MObject
{
public:
    MIScene();
    virtual ~MIScene();

public:

	virtual void SetRootNode(MNode* pRootNode);
	MNode* GetRootNode() { return m_pRootNode; }

	MCamera* GetCamera();


	void OnAddNode(MNode* pNode);
	void OnRemoveNode(MNode* pNode);


	virtual void Render(MIRenderer* pRenderer, MIRenderView* pRenderView);

public:
	virtual void OnCreated() override;

protected:

	void DrawNode(MIRenderer* pRenderer, MNode* pNode);
	void DrawSkyBox(MIRenderer* pRenderer);

private:

	MNode* m_pRootNode;
	MCamera* m_pUsingCamera;
	MCamera* m_pDefaultCamera;

	MSkyBox* m_pSkyBox;

	Matrix4 m_m4CameraInvProj;

	std::vector<MDirectionalLight*> m_vDirectionalLight;
	std::vector<MPointLight*> m_vPointLight;
};


#endif
