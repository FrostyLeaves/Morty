/**
 * @File         MForwardPostProcessProgram
 * 
 * @Created      2020-11-29 15:32:27
 *
 * @Author       DoubleYe
**/

#ifndef _M_MFORWARDPOSTPROCESSPROGRAM_H_
#define _M_MFORWARDPOSTPROCESSPROGRAM_H_
#include "MGlobal.h"
#include "MEngine.h"
#include "MForwardRenderProgram.h"

class MMaterial;
class MRenderPass;
class MForwardHDRWork;
class MIPostProcessWork;
class MIRenderTexture;
class MORTY_API MForwardPostProcessProgram : public MForwardRenderProgram
{
public:
	M_OBJECT(MForwardPostProcessProgram);
    MForwardPostProcessProgram();
    virtual ~MForwardPostProcessProgram();

public:
	virtual void Render(MIRenderer* pRenderer, MViewport* pViewport) override;

	template<typename CLASS_TYPE>
	CLASS_TYPE* AppendPostProcess();


	void SetHighDynamicRangeEnable(const bool& bEnable);
	bool GetHighDynamicRangeEnable() { return m_bHDR_Enable; }

protected:
	virtual void Initialize() override;
	virtual void Release() override;



protected:

	//HDR first
	bool m_bHDR_Enable;
	std::vector<MIPostProcessWork*> m_vPostProcessWork;

	MIMesh* m_pScreenDrawMesh;
	MMaterial* m_pScreenDrawMaterial;
};

template<typename CLASS_TYPE>
CLASS_TYPE* MForwardPostProcessProgram::AppendPostProcess()
{
	CLASS_TYPE* pPostProcess = GetEngine()->GetObjectManager()->CreateObject<CLASS_TYPE>();
	pPostProcess->Initialize(this);

	m_vPostProcessWork.push_back(pPostProcess);

	return pPostProcess;
}

#endif
