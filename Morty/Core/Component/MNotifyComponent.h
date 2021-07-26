/**
 * @File         MNotifyComponent
 * 
 * @Created      2021-07-19 17:03:15
 *
 * @Author       Pobrecito
**/

#ifndef _M_MNOTIFYCOMPONENT_H_
#define _M_MNOTIFYCOMPONENT_H_
#include "MGlobal.h"
#include "MComponent.h"

class MORTY_API MComponentNotifyInfo
{
public:
	struct MComponentNotifyCallFunction
	{
		const MType* pComponentType;
		std::function<void()> function;

		bool operator ==(const MComponentNotifyCallFunction& cf) { return pComponentType == cf.pComponentType; }
	};
public:
	MComponentNotifyInfo() :m_vComponentNotifyCallFunction() {}

	void AddNotifyFunction(const MType* pComponentType, const std::function<void()>& callback);
	void RemoveNotifyFunction(const MType* pComponentType);

public:

	std::vector<MComponentNotifyCallFunction> m_vComponentNotifyCallFunction;
};

class MORTY_API MNotifyComponent : public MComponent
{
public:
    MNotifyComponent();
    virtual ~MNotifyComponent();

public:

	template <class T>
	void RegisterComponentNotify(const MString& strNotifyName, const std::function<void()>& callback)
	{
		RegisterComponentNotify(strNotifyName, T::GetClassType(), callback);
	}

	template <class T>
	void UnregisterComponentNotify(const MString& strNotifyName)
	{
		UnregisterComponentNotify(strNotifyName, T::GetClassType());
	}


    void SendNotify(const MString& strNotify);

private:

	void RegisterComponentNotify(const MString& strNotify, const MType* pComponentType, const std::function<void()>& callback);
	void UnregisterComponentNotify(const MString& strNotifyName, const MType* pComponentType);
private:

	std::map<MString, MComponentNotifyInfo*> m_tComponentNotify;
};


#endif
