/**
 * @File         MGaussianBlurWork
 * 
 * @Created      2020-11-30 17:43:05
 *
 * @Author       DoubleYe
**/

#ifndef _M_MGAUSSIANBLURWORK_H_
#define _M_MGAUSSIANBLURWORK_H_
#include "MGlobal.h"
#include "MObject.h"

#include "MForwardRenderProgram.h"
#include "MStandardPostProcessWork.h"

class MMaterial;
class MShaderParamSet;
class MRenderGraphNode;
class MORTY_API MGaussianBlurWork : public MIPostProcessWork
{
public:
	M_OBJECT(MGaussianBlurWork);
    MGaussianBlurWork();
    virtual ~MGaussianBlurWork();

public:

	void SetBlurRadius(const float& fRadius) { m_fBlurRadius = fRadius; }
	float GetBlurRadius() const { return m_fBlurRadius; }

	void SetIteration(const uint32_t& unIteration) { m_unIteration = unIteration; }
	uint32_t GetIteration() const { return m_unIteration; }


	MString GetGraphNodeName() const { return m_strGraphNodeName; }

public:

	virtual void Initialize(MIRenderProgram* pRenderProgram) override;
	virtual void Release() override;
	
	void Render(MRenderGraphNode* pGraphNode);

protected:

	void UpdateShaderSharedParams(MRenderGraphNode* pGraphNode, MRenderInfo& info);

	void InitializeMesh();
	void ReleaseMesh();

	void InitializeGraph();
	void ReleaseGraph();

    void InitializeMaterial();
    void ReleaseMaterial();

protected:

	MString m_strGraphNodeName;

	MIRenderProgram* m_pRenderProgram;

	MIMesh* m_pScreenDrawMesh;
    MMaterial* m_aMaterial[M_BUFFER_NUM];

private:
	float m_fBlurRadius;
	uint32_t m_unIteration;
};

#endif
