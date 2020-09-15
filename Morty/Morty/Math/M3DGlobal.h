// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#ifdef MATH3D_EXPORTS
#if defined(__WINDOWS_) || defined(_WINDOWS)
#define MATH_IOE_DLL __declspec(dllexport)
#else
#define MATH_IOE_DLL
#endif
#else
#if defined(__WINDOWS_) || defined(_WINDOWS)
#define MATH_IOE_DLL __declspec(dllimport)
#else
#define MATH_IOE_DLL
#endif
#endif


#if defined(__WINDOWS_) || defined(_WINDOWS)


#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>
#endif



// TODO: reference additional headers your program requires here
