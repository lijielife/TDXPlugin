/* Replace "dll.h" with the name of your header */
#include "dll.h"
#include "comm.h"
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char DLL_PATH[500];

char *GetDllPath() {
	return DLL_PATH;
}

DLLIMPORT void HelloWorld ()
{
}

BOOL APIENTRY DllMain (HINSTANCE hInst     /* Library instance handle. */ ,
                       DWORD reason        /* Reason this function is being called. */ ,
                       LPVOID reserved     /* Not used. */ )
{
	GetModuleFileName(hInst, DLL_PATH, sizeof(DLL_PATH));
	char *p = strrchr(DLL_PATH, '\\');
	if (p != NULL) {
		p[1] = 0;
	}
	
    switch (reason)
    {
      case DLL_PROCESS_ATTACH:
        break;

      case DLL_PROCESS_DETACH:
        break;

      case DLL_THREAD_ATTACH:
        break;

      case DLL_THREAD_DETACH:
        break;
    }

    /* Returns TRUE on success, FALSE on failure */
    return TRUE;
}
