/**
 * @File         MStandardPostProcessWork
 * 
 * @Created      2020-11-30 17:58:42
 *
 * @Author       DoubleYe
**/

#ifndef _M_MSTANDARDPOSTPROCESSWORK_H_
#define _M_MSTANDARDPOSTPROCESSWORK_H_
#include "MGlobal.h"
#include "MObject.h"
#include "MIPostProcessWork.h"

#include <array>

class MIMesh;
class MRenderPass;
class MIRenderProgram;
class MIRenderTexture;
class MORTY_API MStandardPostProcessWork : public MIPostProcessWork
{
public:
	M_OBJECT(MStandardPostProcessWork);
	MStandardPostProcessWork();
	virtual ~MStandardPostProcessWork();

public:

	virtual void Initialize(MIRenderProgram* pRenderProgram) override;
	virtual void Release() override;

protected:

	void InitializeMesh();
	void ReleaseMesh();

	void InitializeRenderGraph();
	void ReleaseRenderGraph();

protected:

	MIRenderProgram* m_pRenderProgram;
	MIMesh* m_pScreenDrawMesh;

};
#endif
