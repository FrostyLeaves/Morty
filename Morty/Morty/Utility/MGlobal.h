#ifdef MORTY_EXPORTS
#define MORTY_CLASS __declspec(dllexport)
#else
#define MORTY_CLASS __declspec(dllimport)
#endif

typedef unsigned long MObjectID;

//gles
#define MORTY_OPENGLES 1
//directx11
#define MORTY_DIRECTX_11 2

#define RENDER_GRAPHICS MORTY_DIRECTX_11
