/**
 * @File         MForwardTransparentRenderProgram
 * 
 * @Created      2020-08-25 11:25:01
 *
 * @Author       Pobrecito
**/

#ifndef _M_MFORWARDTRANSPARENTRENDERPROGRAM_H_
#define _M_MFORWARDTRANSPARENTRENDERPROGRAM_H_
#include "MGlobal.h"
#include "MObject.h"
#include "MForwardRenderProgram.h"

class MTransparentRenderTarget;
class MORTY_CLASS MForwardTransparentRenderProgram : public MObject
{
public:
	M_OBJECT(MForwardTransparentRenderProgram);
    MForwardTransparentRenderProgram();
    virtual ~MForwardTransparentRenderProgram();

public:

    void SetProgram(MIRenderProgram* pRenderProgram) { m_pRenderProgram = pRenderProgram; }
    MIRenderProgram* GetProgram() const { return m_pRenderProgram; }

    void DrawTransparentMesh(MForwardRenderProgram::MRenderInfo& info);

    virtual void OnCreated() override;
    virtual void OnDelete() override;


protected:

    void InitializeMesh();
    void ReleaseMesh();

    void InitializeRenderTargets();
    void ReleaseRenderTargets();

	void CheckTransparentTextureSize(MForwardRenderProgram::MRenderInfo& info);
private:

    MIRenderProgram* m_pRenderProgram;

	MTransparentRenderTarget* m_pTransparentRenderTarget0;
	MTransparentRenderTarget* m_pTransparentRenderTarget1;
	MTransparentRenderTarget* m_pTransparentRenderTarget2;

	Vector2 m_v2TransparentTextureSize;
	std::array<MRenderBackTexture*, M_BUFFER_NUM> m_vTransparentFrontTexture;
    std::array<MRenderBackTexture*, M_BUFFER_NUM> m_vTransparentBackTexture;
	std::vector<MRenderBackTexture*> m_vRenderTargetTexture;

	MMesh<Vector2> m_TransparentDrawMesh;
};

#endif
