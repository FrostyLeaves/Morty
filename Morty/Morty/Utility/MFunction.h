/**
 * @File         MFunction
 * 
 * @Created      2019-05-13 00:31:54
 *
 * @Author       DoubleYe
**/

#ifndef _M_MFUNCTION_H_
#define _M_MFUNCTION_H_
#include "MGlobal.h"
#include <map>
#include <functional>


#define M_RETURN_OVER_RANGE(I, MIN, MAX, ...)\
	if (I < MIN || I >= MAX) return ##__VA_ARGS__;


template<typename T1, typename T2>
void DELETE_CLEAR_MAP(std::map<T1, T2>& map)
{
	for (auto iter = map.begin(); iter != map.end(); ++iter)
		delete iter->second;
	
	map.clear();
}

template<typename T>
bool UNION_PUSH_BACK_VECTOR(std::vector<T>& vector, const T& value)
{
	for (T& v : vector)
		if (v == value) return false;
	vector.push_back(value);

	return true;
}

template<typename T>
bool ERASE_FIRST_VECTOR(std::vector<T>& vector, const T& value)
{
	for (auto iter = vector.begin(); iter != vector.end(); ++iter)
	{
		if (*iter == value)
		{
			vector.erase(iter);
			return true;
		}
	}

	return false;
}

template<typename T>
uint32_t UNION_ORDER_PUSH_BACK_VECTOR(std::vector<T>& vector, const T& value, const std::function<bool(const T& a, const T& b)>& lessComp = std::less<T>(), const std::function<bool(const T& a, const T& b)>& equalComp = std::equal_to<T>())
{
	auto iter = std::lower_bound(vector.begin(), vector.end(), value, lessComp);

	if (iter == vector.end())
		vector.push_back(value);
	else if (equalComp(*iter, value))
		return M_INVALID_INDEX;
	else
		vector.insert(iter, value);

	return iter - vector.begin();
}

template<typename T>
void ERASE_UNION_ORDER_VECTOR(std::vector<T>& vector, const T& value, const std::function<bool(const T& a, const T& b)>& lessComp = std::less<T>(), const std::function<bool(const T& a, const T& b)>& equalComp = std::equal_to<T>())
{
	auto iter = std::lower_bound(vector.begin(), vector.end(), value, lessComp);

	if (iter != vector.end() && equalComp(*iter, value))
	{
		vector.erase(iter);
	}
}

#endif
