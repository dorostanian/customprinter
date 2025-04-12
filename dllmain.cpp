// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include <windows.h>
#include <crtdbg.h>
#include <cstdio>
#include <stdlib.h> // Include this header for _set_invalid_parameter_handler
#include "PortMonitor.h" // Include the shared header

#include <string>
CRITICAL_SECTION g_configCriticalSection;
CRITICAL_SECTION g_logCriticalSection;

#ifndef _countof
#define _countof(Array) (sizeof(Array) / sizeof((Array)[0]))
#endif
void MyInvalidParameterHandler(
    const wchar_t* /*expression*/,
    const wchar_t* /*function*/,
    const wchar_t* /*file*/,
    unsigned int /*line*/,
    uintptr_t /*pReserved*/
) {
    OutputDebugStringW(L"[PortMonitor] CRT Invalid parameter encountered.\n");
    if (IsDebuggerPresent()) {
        __debugbreak();
    }
}


extern void LoadConfiguration();
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason, LPVOID lpReserved) {
    switch (ul_reason) {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hModule);
        InitializeCriticalSection(&g_configCriticalSection);
        InitializeCriticalSection(&g_logCriticalSection);
        _set_invalid_parameter_handler(MyInvalidParameterHandler);
        LoadConfiguration();
        OutputDebugStringW(L"[PortMonitor] DLL loaded version 1.0.4\n");
        break;
    case DLL_PROCESS_DETACH:
        DeleteCriticalSection(&g_configCriticalSection);
        DeleteCriticalSection(&g_logCriticalSection);
        break;
    }
    return TRUE;
}
