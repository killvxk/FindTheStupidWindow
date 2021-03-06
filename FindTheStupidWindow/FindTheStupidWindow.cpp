// Find the stupid window
//
// Copyright (C) 2017 David Roller
//
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgement in the product documentation would be
//    appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.

#include "stdafx.h"
#include <Windows.h>
#include <fstream>
#include <time.h>
#include <iostream>
#include <string>
#include <iomanip>
#include <chrono>

#ifdef WIN32
#define localtime_r(_Time, _Tm) localtime_s(_Tm, _Time)
#endif

BOOL QueryWindowFullProcessImageName(
    HWND hwnd,
    DWORD dwFlags,
    PTSTR lpExeName,
    DWORD dwSize)
{
    DWORD pid = 0;
    BOOL fRc = FALSE;
    if (GetWindowThreadProcessId(hwnd, &pid))
    {
        HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
        if (hProcess)
        {
            fRc = QueryFullProcessImageName(hProcess, dwFlags, lpExeName, &dwSize);
            CloseHandle(hProcess);
        }
    }
    return fRc;
}

VOID CALLBACK WinEventProcCallback(
    HWINEVENTHOOK hWinEventHook,
    DWORD event,
    HWND hwnd,
    LONG idObject,
    LONG idChild,
    DWORD dwEventThread,
    DWORD dwmsEventTime)
{
    if (event == EVENT_SYSTEM_FOREGROUND &&
        idObject == OBJID_WINDOW &&
        idChild == CHILDID_SELF)
    {
        PCTSTR pszMsg;
        TCHAR szBuf[MAX_PATH];
        if (hwnd)
        {
            DWORD cch = ARRAYSIZE(szBuf);
            if (QueryWindowFullProcessImageName(hwnd, 0, szBuf, ARRAYSIZE(szBuf)))
            {
                pszMsg = szBuf;
            }
            else
            {
                pszMsg = TEXT("<unknown>");
            }
        }
        else
        {
            pszMsg = TEXT("<none>");
        }

        std::wofstream outfile;
        outfile.open("test.log", std::ios_base::app);

        tm localTime;
        std::chrono::system_clock::time_point t = std::chrono::system_clock::now();
        time_t now = std::chrono::system_clock::to_time_t(t);
        localtime_r(&now, &localTime);

        const std::chrono::duration<double> tse = t.time_since_epoch();
        std::chrono::seconds::rep milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(tse).count() % 1000;

        std::wcout << (1900 + localTime.tm_year) << L'-'
            << std::setfill(L'0') << std::setw(2) << (localTime.tm_mon + 1) << L'-'
            << std::setfill(L'0') << std::setw(2) << localTime.tm_mday << L' '
            << std::setfill(L'0') << std::setw(2) << localTime.tm_hour << L':'
            << std::setfill(L'0') << std::setw(2) << localTime.tm_min << L':'
            << std::setfill(L'0') << std::setw(2) << localTime.tm_sec << L'.'
            << std::setfill(L'0') << std::setw(3) << milliseconds
            << L" - " << pszMsg << std::endl << std::flush;

        outfile << (1900 + localTime.tm_year) << L'-'
            << std::setfill(L'0') << std::setw(2) << (localTime.tm_mon + 1) << L'-'
            << std::setfill(L'0') << std::setw(2) << localTime.tm_mday << L' '
            << std::setfill(L'0') << std::setw(2) << localTime.tm_hour << L':'
            << std::setfill(L'0') << std::setw(2) << localTime.tm_min << L':'
            << std::setfill(L'0') << std::setw(2) << localTime.tm_sec << L'.'
            << std::setfill(L'0') << std::setw(3) << milliseconds
            << L" - " << pszMsg << std::endl << std::flush;
    }
}

int main()
{
    CoInitialize(NULL);

    auto hEvent = SetWinEventHook(EVENT_SYSTEM_FOREGROUND,
        EVENT_SYSTEM_FOREGROUND, NULL,
        WinEventProcCallback, 0, 0,
        WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    UnhookWinEvent(hEvent);
    CoUninitialize();
    return 0;
}
