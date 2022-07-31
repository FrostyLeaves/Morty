/**
 * @File         MNotifySystem
 * 
 * @Created      2021-08-06 13:58:11
 *
 * @Author       Pobrecito
**/

#ifndef _M_MNOTIFYSYSTEM_H_
#define _M_MNOTIFYSYSTEM_H_
#include "Utility/MGlobal.h"
#include "Engine/MSystem.h"
#include "Component/MComponent.h"

typedef std::function<void(MComponent* pComponent)> MNotifyFunction;

class MORTY_API MNotifySystem : public MISystem
{
    MORTY_CLASS(MNotifySystem)

public:
    MNotifySystem();
    virtual ~MNotifySystem();

public:

    void SendNotify(const MString& strNotifyName, MScene* pScene, const MComponentID& senderID);

    void RegisterNotify(const MString& strNotifyName, MNotifyFunction func);
    void UnregisterNotify(const MString& strNotifyName, MNotifyFunction func);

private:

    std::map<MString, std::vector<MNotifyFunction>> m_tNotifyTable;

};


#endif
