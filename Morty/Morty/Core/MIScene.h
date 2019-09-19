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

class MNode;
class MCamera;
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

protected:
	virtual void OnCreated() override;

private:

	MNode* m_pRootNode;
	MCamera* m_pUsingCamera;
	MCamera* m_pDefaultCamera;

};


#endif
