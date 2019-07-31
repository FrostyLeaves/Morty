#ifdef MORTY_EXPORTS
    #ifdef __WINDOWS_
        #define MORTY_CLASS __declspec(dllexport)
    #else
        #define MORTY_CLASS
    #endif
#else
    #ifdef __WINDOWS_
        #define MORTY_CLASS __declspec(dllimport)
    #else
        #define MORTY_CLASS
    #endif
#endif

#ifdef __WINDOWS_
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
