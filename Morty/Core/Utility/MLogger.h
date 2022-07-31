/**
 * @File         MLogger
 * 
 * @Created      2019-05-13 00:38:35
 *
 * @Author       DoubleYe
**/

#ifndef _M_MLOGGER_H_
#define _M_MLOGGER_H_
#include "Utility/MGlobal.h"
#include "Utility/MString.h"

#ifdef MORTY_WIN
#include <windows.h>
#endif

#include <cstdio>
#include <cstdarg>
#include <cstdlib>
#include <cstring>
#include <functional>

class MORTY_API MLogger
{
public:
	MLogger();
	virtual ~MLogger() {};

public:

	typedef std::function<void(const char*)> MLogFunction;
	void SetPrintFunction(MLogFunction func) { m_printFunction = func; }

	template<typename ...ARGS_T>
	void Print(const char* svMessage, ARGS_T ...Args)
	{
		if (m_printFunction) {
			snprintf(svLogData, 4096, svMessage, Args...);
			m_printFunction(svLogData);
		}
		else
		{
			printf(svMessage, Args...);
			printf("\n");
		}

	}

	template<typename ...ARGS_T>
	void Error(const char* svMessage, ARGS_T ...Args)
	{
#ifdef MORTY_WIN
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY |
			FOREGROUND_RED);
#endif

		Print(svMessage, Args...);

#ifdef MORTY_WIN
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY |
			FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
#endif
	}

	template<typename ...ARGS_T>
	void Information(const char* svMessage, ARGS_T... Args)
	{
#ifdef MORTY_WIN
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY |
			FOREGROUND_RED | FOREGROUND_GREEN);

#endif

		Print(svMessage, Args...);

#ifdef MORTY_WIN
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY |
			FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
#endif
	}

	template<typename ...ARGS_T>
	void Log(const char* svMessage, ARGS_T... Args)
	{
#ifdef MORTY_WIN
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY |
			FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
#endif

		Print(svMessage, Args...);
	}

	template<typename ...ARGS_T>
	void Warning(const char* svMessage, ARGS_T... Args)
	{
#ifdef MORTY_WIN
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY |
			FOREGROUND_RED | FOREGROUND_GREEN);
#endif

		Print(svMessage, Args...);

#ifdef MORTY_WIN
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY |
			FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
#endif
	}
private:
	MLogFunction m_printFunction;
	char svLogData[4096];
};


#endif
