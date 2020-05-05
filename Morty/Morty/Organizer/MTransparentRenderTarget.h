/**
 * @File         MTransparentRenderTarget
 * 
 * @Created      2020-05-06 02:09:43
 *
 * @Author       Pobrecito
**/

#ifndef _M_MTRANSPARENTRENDERTARGET_H_
#define _M_MTRANSPARENTRENDERTARGET_H_
#include "MGlobal.h"
#include "MObject.h"
#include "MTextureRenderTarget.h"

#include "MMaterialGroup.h"
#include "MRenderStructure.h"

class MORTY_CLASS MTransparentRenderTarget : public MObject, public MTextureRenderTarget
{
public:
	M_OBJECT(MTransparentRenderTarget);
    MTransparentRenderTarget();
    virtual ~MTransparentRenderTarget();

public:
	virtual void OnCreated() override;
	virtual void OnDelete() override;

	virtual void OnRender(MIRenderer* pRenderer) override;

    
    void CopyFromDepthTextureBuffer(MDepthTextureBuffer* pBuffer);
    void CopyToDepthTextureBuffer(MDepthTextureBuffer* pBuffer);

    void SetSourceMeshes(std::vector<MMaterialGroup>* pGroup) { m_pTransparentMeshes = pGroup; }


    void SetCurrentLevel(const int& nLevel) { m_nCurLevel = nLevel; }
    int GetCurrentLevel() { return m_nCurLevel; }
    int GetLevelNumber() { return m_nLevelNumber; }

private:

    int m_nCurLevel;
    int m_nLevelNumber;

    std::vector<MMaterialGroup>* m_pTransparentMeshes;

};

#endif
