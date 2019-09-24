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
class MIViewport;
class MORTY_CLASS MIScene : public MObject
{
public:
    MIScene();
    virtual ~MIScene();

public:

	virtual void SetRootNode(MNode* pRootNode);
	MNode* GetRootNode() { return m_pRootNode; }

	void OnAddNode(MNode* pNode);
	void OnRemoveNode(MNode* pNode);


public:
	virtual void Render(MIRenderer* pRenderer, MIViewport* pViewport);

	virtual void OnCreated() override;

	void SetAttachedViewport(MIViewport* pViewport);

protected:

	void DrawNode(MIRenderer* pRenderer, MIViewport* pViewport, MNode* pNode);
	void DrawSkyBox(MIRenderer* pRenderer, MIViewport* pViewport);

private:

	MNode* m_pRootNode;
	MSkyBox* m_pSkyBox;

	std::vector<MDirectionalLight*> m_vDirectionalLight;
	std::vector<MPointLight*> m_vPointLight;

	MIViewport* m_pAttachedViewport;
};


#endif
