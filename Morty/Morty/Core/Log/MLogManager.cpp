#include "MLogManager.h"
#include <cstdio>
#include <cstdarg>
#include <cstdlib>
#include <cstring>

#ifdef MORTY_WIN
#include <windows.h>
#endif

void MLogManager::Error(const char* svMessage, ...)
{
    
#ifdef MORTY_WIN
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY |
		FOREGROUND_RED);
#endif

	va_list args;
	va_start(args, svMessage);
	vprintf(svMessage, args);
	printf("\n");
	va_end(args);
}

void MLogManager::Information(const char* svMessage, ...)
{
#ifdef MORTY_WIN
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY |
		FOREGROUND_RED | FOREGROUND_GREEN);
#endif
    
	va_list args;
	va_start(args, svMessage);
	vprintf(svMessage, args);
	printf("\n");
	va_end(args);
}

void MLogManager::Log(const char* svMessage, ...)
{
#ifdef MORTY_WIN
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY |
		FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
#endif
    
	va_list args;
	va_start(args, svMessage);
	vprintf(svMessage, args);
	printf("\n");
	va_end(args);
}

void MLogManager::Warning(const char* svMessage, ...)
{
#ifdef MORTY_WIN
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY |
		FOREGROUND_RED | FOREGROUND_GREEN);
#endif
    
	va_list args;
	va_start(args, svMessage);
	vprintf(svMessage, args);
	printf("\n");
	va_end(args);
}
