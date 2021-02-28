/**
 * @File         MCombineWork
 * 
 * @Created      2020-11-30 17:58:42
 *
 * @Author       DoubleYe
**/

#ifndef _M_MCOMBINEWORK_H_
#define _M_MCOMBINEWORK_H_
#include "MGlobal.h"
#include "MObject.h"
#include "MStandardPostProcessWork.h"

#include <array>

class MMaterial;
class MORTY_API MCombineWork : public MIPostProcessWork
{
public:
	M_OBJECT(MCombineWork);
	MCombineWork();
	virtual ~MCombineWork();

public:

	virtual void Initialize(MIRenderProgram* pRenderProgram) override;
	virtual void Release() override;

	void Render(MRenderGraphNode* pGraphNode);

	MString GetGraphNodeName() const { return m_strGraphNodeName; }

protected:

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

};
#endif
