#ifndef _M_NOTIFY_SUB_SYSTEM_H_
#define _M_NOTIFY_SUB_SYSTEM_H_

#include "Utility/MGlobal.h"
#include "Type/MType.h"
#include "Scene/MSubSystem.h"

class MScene;
class MEngine;
class MORTY_API MNotifySubSystem : public MISubSystem
{
public:
	MORTY_INTERFACE(MNotifySubSystem)

public:

	MNotifySubSystem();
	virtual ~MNotifySubSystem();

public:


};

#endif