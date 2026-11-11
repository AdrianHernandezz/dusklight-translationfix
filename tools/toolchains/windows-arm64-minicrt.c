// Minimal DLL startup for Windows ARM64 mods built without the MSVC runtime libraries.
// Runs C/C++ static initializers on load and atexit handlers on unload; uses the UCRT for the rest.
typedef int BOOL; typedef unsigned long DWORD; typedef void* HINSTANCE; typedef void* LPVOID;
#define WINAPI __stdcall
#define TRUE 1
#define FALSE 0
#define DLL_PROCESS_ATTACH 1
#define DLL_PROCESS_DETACH 0
__declspec(dllimport) BOOL WINAPI DisableThreadLibraryCalls(HINSTANCE);
typedef void (__cdecl *_PVFV)(void);
typedef int  (__cdecl *_PIFV)(void);
#pragma section(".CRT$XIA", long, read)
#pragma section(".CRT$XIZ", long, read)
#pragma section(".CRT$XCA", long, read)
#pragma section(".CRT$XCZ", long, read)
__declspec(allocate(".CRT$XIA")) _PIFV __xi_a[] = {0};
__declspec(allocate(".CRT$XIZ")) _PIFV __xi_z[] = {0};
__declspec(allocate(".CRT$XCA")) _PVFV __xc_a[] = {0};
__declspec(allocate(".CRT$XCZ")) _PVFV __xc_z[] = {0};
#pragma comment(linker, "/merge:.CRT=.rdata")

static _PVFV g_atexit[256];
static int g_atexit_n;
int __cdecl atexit(_PVFV f) { if (g_atexit_n >= 256) return -1; g_atexit[g_atexit_n++] = f; return 0; }
int _fltused = 0;

BOOL WINAPI _DllMainCRTStartup(HINSTANCE inst, DWORD reason, LPVOID reserved) {
    (void)reserved;
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(inst);
        for (_PIFV* p = __xi_a; p < __xi_z; ++p) if (*p && (*p)() != 0) return FALSE;
        for (_PVFV* p = __xc_a; p < __xc_z; ++p) if (*p) (*p)();
    } else if (reason == DLL_PROCESS_DETACH) {
        while (g_atexit_n > 0) g_atexit[--g_atexit_n]();
    }
    return TRUE;
}
