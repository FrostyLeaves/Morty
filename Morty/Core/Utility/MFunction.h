/**
 * @File         MFunction
 * 
 * @Created      2019-05-13 00:31:54
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"

MORTY_SPACE_BEGIN

#define M_RETURN_OVER_RANGE(I, MIN, MAX, ...)\
	if (I < MIN || I >= MAX) return ##__VA_ARGS__;

#define M_CLASS_FUNCTION_BIND_0_0(CLASS_FUNC, SELF) std::bind(&CLASS_FUNC, SELF)
#define M_CLASS_FUNCTION_BIND_0_1(CLASS_FUNC, SELF) std::bind(&CLASS_FUNC, SELF, std::placeholders::_1)
#define M_CLASS_FUNCTION_BIND_0_2(CLASS_FUNC, SELF) std::bind(&CLASS_FUNC, SELF, std::placeholders::_1, std::placeholders::_2)

#define M_CLASS_FUNCTION_BIND_1_0(CLASS_FUNC, SELF, PARAM_0) std::bind(&CLASS_FUNC, SELF, PARAM_0)
#define M_CLASS_FUNCTION_BIND_1_1(CLASS_FUNC, SELF, PARAM_0) std::bind(&CLASS_FUNC, SELF, PARAM_0, std::placeholders::_1)


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
bool UNION_PUSH_BACK_VECTOR(std::vector<T>& vector, const T& value, const std::function<bool(const T& a, const T& b)>& equalComp)
{
	for (T& v : vector)
		if (equalComp(v, value)) return false;
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
bool ERASE_FIRST_VECTOR(std::vector<T>& vector, const T& value, const std::function<bool(const T& a, const T& b)>& equalComp)
{
	for (auto iter = vector.begin(); iter != vector.end(); ++iter)
	{
		if (equalComp(*iter, value))
		{
			vector.erase(iter);
			return true;
		}
	}

	return false;
}

template<typename T>
size_t UNION_ORDER_PUSH_BACK_VECTOR(std::vector<T>& vector, const T& value, const std::function<bool(const T& a, const T& b)>& lessComp = std::less<T>(), const std::function<bool(const T& a, const T& b)>& equalComp = std::equal_to<T>())
{
	if (vector.empty())
	{
		vector.push_back(value);
		return 0;
	}

	auto iter = std::lower_bound(vector.begin(), vector.end(), value, lessComp);

	if (iter == vector.end())
	{
		vector.push_back(value);
		return vector.size() - 1;
	}
	else if (equalComp(*iter, value))
		return MGlobal::M_INVALID_INDEX;
	else
	{
		iter = vector.insert(iter, value);
		return iter - vector.begin();
	}

	return MGlobal::M_INVALID_INDEX;
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

template<typename T, typename VT = T>
size_t FIND_ORDER_VECTOR(std::vector<T>& vector, const VT& value, const std::function<bool(const T& a, const VT& b)>& lessComp)
{
	auto iter = std::lower_bound(vector.begin(), vector.end(), value, lessComp);

	return iter - vector.begin();
}

MORTY_SPACE_END