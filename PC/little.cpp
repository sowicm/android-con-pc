
#include <WS2tcpip.h>
#include <Windows.h>
#include <time.h>
#include <process.h>

#include <KMP.h>

#include "resource.h"
#include "common.h"
#include "little.h"
#include "autorun.h"

#pragma comment(lib, "GdiPlus.lib")
#pragma comment(lib, "ws2_32.lib")


//---------------------------------------------------

#define WNDCLASSNAME L"Sowicm's Little"
#define Dllfn      0
#define Force      0

#if Dllfn
typedef BOOL (WINAPI* lpfnUpdateLayeredWindow)(HWND hwnd, HDC hdcDst, POINT *pptDst, SIZE *psize, HDC hdcSrc, POINT *pptSrc, COLORREF crKey, BLENDFUNCTION *pblend, DWORD dwFlags);
lpfnUpdateLayeredWindow updateLayeredWindow;
#endif

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

#if 0
void CALLBACK TimerProc(HWND hWnd, UINT message, UINT uID, DWORD dwTime)
{
    switch (uID)
    {
    case 2:
        g_gif1->next();
        render();
        break;
    }
}
#endif

// ----------------- Begin ------------------

#if 0
void turnGIF()
{
    static bool show = false;
    show = !show;
    if (show)
    {
        g_renderObjects.insert(g_gif1);
        SetTimer(g_hWnd, 2, 200, TimerProc);
    }
    else
    {
        KillTimer(g_hWnd, 2);
        g_renderObjects.erase(g_gif1);
        render();
    }
}
#endif

// ----------------- End --------------------

void little::init(HINSTANCE hInst)
{
    m_hInst = hInst;
    initWindow();
    initGdiplus();
    initRes();
    initUrl();
    m_hWorkThread = nullptr;
    m_hSuperviseThread = nullptr;
    m_hScreenThread = nullptr;
}

void little::initWindow()
{
    RECT& rc = m_rcDesktop;
    GetWindowRect(GetDesktopWindow(), &m_rcDesktop);
    m_rect.x       = rc.left;
    m_rect.y       = rc.top;
    m_rect.width   = rc.right - rc.left;
    m_rect.height  = rc.bottom - rc.top;
    m_cyFullScreen = GetSystemMetrics(SM_CYFULLSCREEN);
    
    WNDCLASSEXW wndCls;
    wndCls.cbSize        = sizeof(wndCls);
    wndCls.hInstance     = m_hInst;
    wndCls.lpszClassName = WNDCLASSNAME;
    wndCls.style         = CS_HREDRAW | CS_VREDRAW;
    wndCls.lpfnWndProc   = WndProc;
    wndCls.hIcon         = NULL; // todo
    wndCls.hIconSm       = NULL; // todo
    wndCls.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wndCls.cbClsExtra    = 0;
    wndCls.cbWndExtra    = 0;
    wndCls.hbrBackground = NULL;
    wndCls.lpszMenuName  = NULL;

    RegisterClassExW(&wndCls);

    m_hWnd = CreateWindowExW(WS_EX_LAYERED |
                             WS_EX_TOPMOST |
                             WS_EX_TOOLWINDOW|  // 隐藏任务栏图标
                             WS_EX_TRANSPARENT, // 鼠标穿透
                                WNDCLASSNAME,
                                NULL,
                                WS_OVERLAPPEDWINDOW,
                                m_rect.x,
                                m_rect.y,
                                m_rect.width,
                                m_rect.height,
                                NULL,
                                NULL,
                                m_hInst,
                                NULL);
}

void little::initGdiplus()
{
    GdiplusStartupInput gdiplusStartupInput;
    

    GdiplusStartup(&m_gdiplusToken, &gdiplusStartupInput, NULL);

    ShowWindow(m_hWnd, SW_NORMAL);
    //SetLayeredWindowAttributes(hWnd, 0, (255 * 70) / 100, LWA_ALPHA);
    
    m_screenDC = GetDC(NULL);
    HBITMAP bmp;
    BITMAPINFO bmi = {0};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = m_rect.width;
    bmi.bmiHeader.biHeight = m_rect.height;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    bmp = CreateDIBSection(NULL, &bmi, DIB_RGB_COLORS, NULL, 0, 0);

    m_memDC = CreateCompatibleDC(m_screenDC);

    SelectObject(m_memDC, bmp);

    m_pGraphics = Graphics::FromHDC(m_memDC);
}

void little::initRes()
{
    int &w = m_rect.width, &y = m_cyFullScreen;
    //g_gif1 = new GIFObject(NULL, IDR_GIF1, g_rect.width - 138, g_rect.height - 185);
}

bool little::getID(char* str)
{
    addrinfo hints;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    addrinfo *pai;
    if (getaddrinfo("my.zi-jin.com", "80", &hints, &pai) != 0)
        Sleep(2000);

    SOCKET sck = socket(AF_INET, SOCK_STREAM, 0);
    if (INVALID_SOCKET == sck)
        return false;

    try
    {
        if (connect(sck, pai->ai_addr, pai->ai_addrlen) == SOCKET_ERROR)
            throw L"";

        const char strHeader[] = "GET "LittlePath"sign.php HTTP/1.1\r\nHost: my.zi-jin.com\r\nConnection: Close\r\n\r\n";
        if (send(sck, strHeader, strlen(strHeader), 0) == SOCKET_ERROR)
            throw L"";
    }
    catch (...)
    {
        closesocket(sck);
        return false;
    }

    char buf[2048];
    int received = recv(sck, buf, 2047, 0);
    buf[received] = '\0';

    closesocket(sck);
    
    int i;
    i = KMP(buf, "\r\n\r\n", 0, true);
    if (i < 0)
        return false;
    while (buf[i++] != '[');
    strcpy(str, buf + i);
    i = -1;
    while (str[++i] != ']');
    str[i] = '\0';
    return true;
}

void little::initUrl()
{
    char str[128] = LittlePath;
    char cid[33];

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData))
        return;

    while (!getID(cid))
        Sleep(500);

    strcat(str, "c/");
    strcat(str, cid);
    url.fromString(str);
    
    WSACleanup();
    
}

void little::startWorkThread()
{
    bWorkThreadRun = true;
    m_hWorkThread = (HANDLE)_beginthread(workthreadStart, 0, nullptr);
    nWorkThreadWorkClock = clock();
}

void little::startSuperviseThread()
{
    m_hSuperviseThread = (HANDLE)_beginthread(supervisethreadStart, 0, nullptr);
}

void little::startScreenThread()
{
    bScreenThreadRun = true;
    m_hScreenThread = (HANDLE)_beginthread(screenthreadStart, 0, nullptr);
}

void little::release()
{
    delete m_pGraphics;
    GdiplusShutdown(m_gdiplusToken);
}




little app;

int APIENTRY wWinMain(HINSTANCE hInst,
                      HINSTANCE hPrevInst,
                      LPWSTR    lpCmdLine,
                      int       nCmdShow)
{
#if Dllfn
    HMODULE hModule = GetModuleHandleW(L"user32.dll");
    updateLayeredWindow = (lpfnUpdateLayeredWindow)GetProcAddress(hModule, "UpdateLayeredWindow");
#endif

#ifndef DEBUG
    setAutoRun();
#endif

    MessageBoxW(NULL, L"U盘已解锁！", L"Unlock", MB_OK);

    app.init(hInst);
    app.startWorkThread();
#ifndef DEBUG
    app.startSuperviseThread();
#endif
    
    MSG msg;

  #if Force
    while (true) {
  #endif
      while (GetMessage(&msg, NULL, 0, 0))
      {
          TranslateMessage(&msg);
          DispatchMessage(&msg);
      }
  #if Force
    }
  #endif

    app.release();

    return msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_PAINT:
        return 0;
#if Force
    case WM_QUIT:
    case WM_CLOSE:
    case WM_DESTROY:
        return 0;
#endif
    }
    return DefWindowProcW(hWnd, message, wParam, lParam);
}