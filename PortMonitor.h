// PortMonitor.h
#pragma once
#include <windows.h>
// Declare critical sections as extern to share across files
extern CRITICAL_SECTION g_configCriticalSection;
extern CRITICAL_SECTION g_logCriticalSection;