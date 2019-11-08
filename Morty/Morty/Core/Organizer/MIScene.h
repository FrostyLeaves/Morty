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
class MMaterial;
class MMeshInstance;
class MORTY_CLASS MIScene : public MObject
{
public:
	M_OBJECT(MIScene);
    MIScene();
    virtual ~MIScene();

public:

	virtual void SetRootNode(MNode* pRootNode);
	MNode* GetRootNode() { return m_pRootNode; }

public:

	void FindActivePointLights(const Vector3& v3WorldPosition, std::vector<MPointLight*>& vPointLights);
public:


	//节点接入场景时进行的操作
	void OnNodeEnter(MNode* pNode);
	//节点退出场景时进行的操作
	void OnNodeExit(MNode* pNode);

	void RecordMeshInstance(MMeshInstance* pMeshInstance);
	void CancelRecordMeshInstance(MMeshInstance* pMeshInstance);

	virtual void Render(MIRenderer* pRenderer, MIViewport* pViewport);

	virtual void OnCreated() override;

	void AddAttachedViewport(MIViewport* pViewport);
	void RemoveAttachedViewport(MIViewport* pViewport);
	std::vector<MIViewport*> GetViewports() { return m_vViewports; }

protected:

	void DrawMeshInstance(MIRenderer* pRenderer, MIViewport* pViewport);
	void DrawSkyBox(MIRenderer* pRenderer, MIViewport* pViewport);
	void DrawPainter(MIRenderer* pRenderer, MIViewport* pViewport);

private:

	MNode* m_pRootNode;
	MSkyBox* m_pSkyBox;

	std::vector<MDirectionalLight*> m_vDirectionalLight;
	std::vector<MPointLight*> m_vPointLight;

	struct MaterialMeshInsGroup
	{
		MMaterial* pMat;
		std::vector<MMeshInstance*> vMeshIns;
	};
	std::vector<MaterialMeshInsGroup*> m_vMatMeshInsGroup;

	std::vector<MIViewport*> m_vViewports;
};


#endif
