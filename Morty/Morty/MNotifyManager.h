/**
 * @File         MObject
 * 
 * @Created      2019-07-29 20:26:33
 *
 * @Author       Pobrecito
**/

#ifndef _M_MNOTIFY_MANAGER_H_
#define _M_MNOTIFY_MANAGER_H_
#include "MGlobal.h"

#include "MObject.h"
#include "MString.h"

#include <map>
#include <vector>
#include <functional>

class MNotifyManager
{
public:
	typedef std::function<void(MObject* pSender, MObject* pData)> MNotifyEventFunction;

	bool RegisterNotify(const MString& strNotifyName, MObject* pObject, MNotifyEventFunction function);

	void UnRegisterNotify(const MString& strNotifyName, MObject* pObject);

	void UnRegisterAllNotifyByObject(MObject* pObject);

private:


	struct NotifyRegister
	{
		NotifyRegister(const MString& strNotifyName, const MNotifyEventFunction& func) : strNotifyName(strNotifyName), function(func){}
		MString strNotifyName;
		MNotifyEventFunction function;
	};

	std::map<MObject*, std::vector<NotifyRegister>* > m_tNotifyMap;
};


#endif
