#include "MLogManager.h"
#include <cstdio>
#include <cstdarg>
#include <cstdlib>
#include <cstring>
#include <windows.h>

void MLogManager::Error(const char* svMessage, ...)
{
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY |
		FOREGROUND_RED);

	va_list args;
	va_start(args, svMessage);
	vprintf(svMessage, args);
	printf("\n");
	va_end(args);
}

void MLogManager::Information(const char* svMessage, ...)
{
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY |
		FOREGROUND_RED | FOREGROUND_GREEN);

	va_list args;
	va_start(args, svMessage);
	vprintf(svMessage, args);
	printf("\n");
	va_end(args);
}

void MLogManager::Log(const char* svMessage, ...)
{
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY |
		FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);

	va_list args;
	va_start(args, svMessage);
	vprintf(svMessage, args);
	printf("\n");
	va_end(args);
}

void MLogManager::Warning(const char* svMessage, ...)
{
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY |
		FOREGROUND_RED | FOREGROUND_GREEN);

	va_list args;
	va_start(args, svMessage);
	vprintf(svMessage, args);
	printf("\n");
	va_end(args);
}
