/**
 * @File         MLogger
 * 
 * @Created      2019-05-13 00:38:35
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"
#include "Utility/MString.h"

#ifdef MORTY_WIN

#include <windows.h>

#endif

#include <cstdio>
#include <functional>

#include "fmt/format.h"

namespace morty
{

enum class MLogType
{
    EDefault = 0,
    EInfo,
    EWarn,
    EError,
};

class MORTY_API MLogger
{
public:
    MLogger() = default;

    ~MLogger() = default;

public:
    typedef std::function<void(MLogType eType, const char*)> MLogFunction;

    void SetPrintFunction(MLogFunction func) { m_printFunction = func; }

    template<typename... ARGS_T>
    void Print(MLogType eType, const char* svMessage, ARGS_T&&... Args)
    {
        auto logData = fmt::vformat(svMessage, fmt::make_format_args(Args...));

        if (m_printFunction) { m_printFunction(eType, logData.c_str()); }
        else { printf("%s\n", logData.c_str()); }
    }

    template<typename... ARGS_T> void Error(const char* svMessage, ARGS_T&&... Args)
    {
#ifdef MORTY_WIN
        SetConsoleTextAttribute(
                GetStdHandle(STD_OUTPUT_HANDLE),
                FOREGROUND_INTENSITY | FOREGROUND_RED
        );
#endif

        Print(MLogType::EError, svMessage, Args...);

#ifdef MORTY_WIN
        SetConsoleTextAttribute(
                GetStdHandle(STD_OUTPUT_HANDLE),
                FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE
        );
#endif
    }

    template<typename... ARGS_T> void Information(const char* svMessage, ARGS_T&&... Args)
    {
#ifdef MORTY_WIN
        SetConsoleTextAttribute(
                GetStdHandle(STD_OUTPUT_HANDLE),
                FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN
        );

#endif

        Print(MLogType::EInfo, svMessage, Args...);

#ifdef MORTY_WIN
        SetConsoleTextAttribute(
                GetStdHandle(STD_OUTPUT_HANDLE),
                FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE
        );
#endif
    }

    template<typename... ARGS_T> void Log(const char* svMessage, ARGS_T&&... Args)
    {
#ifdef MORTY_WIN
        SetConsoleTextAttribute(
                GetStdHandle(STD_OUTPUT_HANDLE),
                FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE
        );
#endif

        Print(MLogType::EDefault, svMessage, Args...);
    }

    template<typename... ARGS_T> void Warning(const char* svMessage, ARGS_T&&... Args)
    {
#ifdef MORTY_WIN
        SetConsoleTextAttribute(
                GetStdHandle(STD_OUTPUT_HANDLE),
                FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN
        );
#endif

        Print(MLogType::EWarn, svMessage, Args...);

#ifdef MORTY_WIN
        SetConsoleTextAttribute(
                GetStdHandle(STD_OUTPUT_HANDLE),
                FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE
        );
#endif
    }

private:
    MLogFunction m_printFunction = nullptr;
};

}// namespace morty