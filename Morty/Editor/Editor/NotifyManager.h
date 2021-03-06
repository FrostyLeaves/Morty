#ifndef _NOTIFY_MANAGER_H_
#define _NOTIFY_MANAGER_H_

#include <map>
#include <functional>

#include "MString.h"
#include "MVariant.h"
#include "MSingleInstance.h"

#define NOTIFY_FUNC(PTR, CLASS_FUNC) (std::bind(&CLASS_FUNC, PTR, std::placeholders::_1))

class IBaseView;
class NotifyManager : public MSingleInstance<NotifyManager>
{
public:
	NotifyManager();
	~NotifyManager();


	typedef std::function<void(const MVariant&)> NotifyFunction;


public:

	void SendNotify(const MString& strNotifyName, const MVariant& param = MVariant());

	void RegisterNotify(const MString& strNotifyName, void* pRegister, const NotifyFunction& func);


	void UnRegisterAll();

private:

	struct NotifyGroup
	{
		std::vector<void*> m_vRegister;
		std::vector<NotifyFunction> m_vFunction;
	};

	std::map<MString, NotifyGroup*> m_tNotifyTable;
};

#endif