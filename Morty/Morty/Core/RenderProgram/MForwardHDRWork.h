/**
 * @File         MForwardHDRWork
 * 
 * @Created      2020-11-29 11:05:51
 *
 * @Author       Pobrecito
**/

#ifndef _M_MFORWARDHDRWORK_H_
#define _M_MFORWARDHDRWORK_H_
#include "MGlobal.h"
#include "MObject.h"
#include "MForwardRenderProgram.h"

#include "MMesh.h"

class MIRenderTarget;
class MIRenderBackTexture;
class MRenderDepthTexture;
class MTextureRenderTarget;
class MORTY_CLASS MForwardHDRWork : public MObject
{
public:
	M_OBJECT(MForwardHDRWork);
    MForwardHDRWork();
    virtual ~MForwardHDRWork();

public:

	void Initialize(MIRenderProgram* pRenderProgram);
	void Release();

	MIRenderTarget* GetTempRenderTarget();

	void CheckTextureSize(MRenderInfo& info);

    void Render(MRenderInfo& info);

public:

	void InitializeMesh();
	void ReleaseMesh();

	void InitializeRenderTargets();
	void ReleaseRenderTargets();

	void InitializeRenderPass();
	void ReleaseRenderPass();

	void InitializeMaterial();
	void ReleaseMaterial();


private:

	MIRenderProgram* m_pRenderProgram;

	std::array<MIRenderBackTexture*, M_BUFFER_NUM> m_aBackTexture;
	std::array<MRenderDepthTexture*, M_BUFFER_NUM> m_aDepthTexture;

	MTextureRenderTarget* m_pTempRenderTarget;
	MRenderPass m_HDRRenderPass;
	
	MMesh<Vector2> m_ScreenDrawMesh;
	MMaterial* m_pHDRMaterial;


};

#endif
