#ifdef MORTY_EXPORTS
#if defined(__WINDOWS_) || defined(_WINDOWS)
        #define MORTY_CLASS __declspec(dllexport)
    #else
        #define MORTY_CLASS
    #endif
#else
	#if defined(__WINDOWS_) || defined(_WINDOWS)
        #define MORTY_CLASS __declspec(dllimport)
    #else
        #define MORTY_CLASS
    #endif
#endif

#if defined(__WINDOWS_) || defined(_WINDOWS)
	#define MORTY_WIN
#else
	#define MORTY_MAC
#endif

typedef unsigned long MObjectID;

//gles
#define MORTY_OPENGLES 1
//directx11
#define MORTY_DIRECTX_11 2

#define RENDER_GRAPHICS MORTY_OPENGLES
