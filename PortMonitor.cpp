#include "pch.h"
#define PORTMONITOR_EXPORTS
#include <windows.h>
#include <string>
#include <mutex>
#include <vector>
#include <shlobj.h>
#include "PortMonitor.h"
std::wstring g_TargetPrinterName;            

struct PORT_CONTEXT {
    bool interceptJob;
    DWORD jobId;
    std::wstring printerName;
    std::vector<BYTE> accumulatedData;
};

void LogMessage(const wchar_t* message) {
    OutputDebugStringW(message);
}

void LoadConfiguration() {
    EnterCriticalSection(&g_configCriticalSection);

    HKEY hKey;
    LSTATUS result = RegOpenKeyExW(
        HKEY_LOCAL_MACHINE,
        L"SOFTWARE\\customprinter",
        0,
        KEY_READ,
        &hKey
    );

    if (result == ERROR_SUCCESS) {
        WCHAR buffer[256] = {0};
        DWORD bufferSize = sizeof(buffer);
        DWORD type = 0;

        result = RegGetValueW(
            hKey,
            nullptr,
            L"TargetPrinterName",
            RRF_RT_REG_SZ,
            &type,
            buffer,
            &bufferSize
        );

        if (result == ERROR_SUCCESS && type == REG_SZ) {
            g_TargetPrinterName = buffer;
        } else {
            g_TargetPrinterName.clear();
        }

        RegCloseKey(hKey);
    }

    LeaveCriticalSection(&g_configCriticalSection);
}

bool SaveBufferToFile(const std::vector<BYTE>& data, DWORD jobId) {
    WCHAR tempDir[MAX_PATH];
    DWORD len = GetTempPathW(MAX_PATH, tempDir);
    if (len == 0 || len >= MAX_PATH) {
        LogMessage(L"[PortMonitor] Error: GetTempPathW failed or buffer too small\n");
        return false;
    }

    WCHAR filename[MAX_PATH];
    int formatResult = _snwprintf_s(filename, MAX_PATH, _TRUNCATE, L"%sintercepted_job_%d.raw", tempDir, jobId);
    if (formatResult < 0) {
        LogMessage(L"[PortMonitor] Error: _snwprintf_s failed to format filename.\n");
        return false;
    }

    HANDLE hFile = CreateFileW(filename, GENERIC_WRITE, 0, nullptr,
        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hFile == INVALID_HANDLE_VALUE) {
        LogMessage(L"[PortMonitor] Error: CreateFileW failed for\n");
        return false;
    }

    DWORD bytesWritten;
    BOOL writeResult = WriteFile(hFile, data.data(),
        static_cast<DWORD>(data.size()), &bytesWritten, nullptr);
    CloseHandle(hFile);

    if (!writeResult || bytesWritten != data.size()) {
        LogMessage(L"[PortMonitor] Error: WriteFile failed for\n");
    }
    return writeResult && (bytesWritten == data.size());
}

extern "C" __declspec(dllexport) BOOL APIENTRY OpenPort(LPWSTR pPortName, PHANDLE phPort) {
    try {
        PORT_CONTEXT* ctx = new PORT_CONTEXT{ false, 0, L"", {} };
        *phPort = reinterpret_cast<HANDLE>(ctx);
        return TRUE;
    }
    catch (...) {
        return FALSE;
    }
}

extern "C" __declspec(dllexport) BOOL APIENTRY WritePort(
    HANDLE hPort, LPBYTE pBuffer, DWORD cbBuffer, LPDWORD pcbWritten
) {
    PORT_CONTEXT* ctx = reinterpret_cast<PORT_CONTEXT*>(hPort);
    if (!ctx || !pBuffer || !pcbWritten) {
        LogMessage(L"[PortMonitor] WritePort: Invalid parameters\n");
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    *pcbWritten = cbBuffer;

    if (ctx->interceptJob) {
        EnterCriticalSection(&g_logCriticalSection);
        ctx->accumulatedData.insert(
            ctx->accumulatedData.end(),
            pBuffer,
            pBuffer + cbBuffer
        );
        LeaveCriticalSection(&g_logCriticalSection);
    }

    return TRUE;
}

extern "C" __declspec(dllexport) BOOL APIENTRY EndDocPort(HANDLE hPort) {
    PORT_CONTEXT* ctx = reinterpret_cast<PORT_CONTEXT*>(hPort);
    if (ctx->interceptJob && !ctx->accumulatedData.empty()) {
        SaveBufferToFile(ctx->accumulatedData, ctx->jobId);
        ctx->accumulatedData.clear();
    }
    return TRUE;
}

extern "C" __declspec(dllexport) BOOL APIENTRY ClosePort(HANDLE hPort) {
    delete reinterpret_cast<PORT_CONTEXT*>(hPort);
    return TRUE;
}

extern "C" __declspec(dllexport) BOOL APIENTRY EnumPorts(
    LPWSTR pName, DWORD Level, LPBYTE pPorts, DWORD cbBuf, LPDWORD pcbNeeded, LPDWORD pcReturned
) {
    return FALSE;
}

extern "C" __declspec(dllexport) BOOL APIENTRY XcvData(
    HANDLE hXcv, LPCWSTR pszDataName, PBYTE pInputData, DWORD cbInputData,
    PBYTE pOutputData, DWORD cbOutputData, PDWORD pcbOutputNeeded, PDWORD pdwStatus
) {
    *pdwStatus = ERROR_CALL_NOT_IMPLEMENTED;
    return FALSE;
}