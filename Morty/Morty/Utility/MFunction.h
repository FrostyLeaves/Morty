/**
 * @File         MFunction
 * 
 * @Created      2019-05-13 00:31:54
 *
 * @Author       Pobrecito
**/

#ifndef _M_MFUNCTION_H_
#define _M_MFUNCTION_H_
#include "MGlobal.h"

template<typename T1, typename T2>
void DELETE_CLEAR_MAP(std::map<T1, T2>& map)
{
	for (std::map<T1, T2>::iterator iter = map.begin(); iter != map.end(); ++iter)
		delete iter->second;
	
	map.clear();
}

template<typename T>
void UNION_PUSH_BACK_VECTOR(std::vector<T>& vector, const T& value)
{
	for (T& v : vector)
		if (v == value) return;
	vector.push_back(value);
}

template<typename T>
void ERASE_FIRST_VECTOR(std::vector<T>& vector, const T& value)
{
	for (std::vector<T>::iterator iter = vector.begin(); iter != vector.end(); ++iter)
	{
		if (*iter == value)
		{
			vector.erase(iter);
			return;
		}
	}
}


#endif
