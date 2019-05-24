/**
* @File         MSingleInstance
*
* @Created      2019-05-12 22:09:30
*
* @Author       Morty
**/

#ifndef _M_SINGLEINSTANCE_H_
#define _M_SINGLEINSTANCE_H_

template <class T>
class MSingleInstance
{
public:
	static T* GetInstance()
	{
		static T inst;
		return &inst;
	}
};






#endif