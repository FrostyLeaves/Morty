/**
 * @File         MComponent
 * 
 * @Created      2021-04-26 16:26:56
 *
 * @Author       Pobrecito
**/

#ifndef _M_MCOMPONENT_H_
#define _M_MCOMPONENT_H_
#include "MGlobal.h"
#include "MTypedClass.h"

#include "MSerializer.h"

class MNode;
class MScene;
class MEngine;
class MORTY_API MComponent : public MTypedClass, public MSerializer
{
public:
    M_OBJECT(MComponent)

public:
    MComponent();
    virtual ~MComponent();

public:

    virtual void Initialize() {}
    virtual void Release() {}

    virtual void SetOwnerNode(MNode* pNode);
    MNode* GetOwnerNode() { return m_pOwnerNode; }

    MScene* GetScene();

    MEngine* GetEngine();

public:

    void SendComponentNotify(const MString& strSignalName);

public:

    virtual void Tick(const float& fDelta) {}

    virtual void PreRender() {}

    virtual void PostRender() {}

public:


	virtual void WriteToStruct(MStruct& srt) override;

    virtual void ReadFromStruct(const MStruct& srt) override {}

private:

    MNode* m_pOwnerNode;
};


#endif
