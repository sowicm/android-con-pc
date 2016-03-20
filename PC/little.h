
#ifndef little_h
#define little_h

#include <GdiPlus.h>
using namespace Gdiplus;

#include <SRect.h>
#include <sUrl.h>

struct addrinfo;

class little
{
public:
    HINSTANCE           m_hInst;
    ULONG_PTR           m_gdiplusToken;
    Graphics* m_pGraphics;
    SRect     m_rect;
    int       m_cyFullScreen;
    HDC       m_screenDC, m_memDC;
    RECT      m_rcDesktop;
    HWND      m_hWnd;
    sUrl      url;
    addrinfo *pai;

    void init(HINSTANCE hInst);
    void initWindow();
    void initGdiplus();
    void initRes();
    void initUrl();
    void startWorkThread();
    void startSuperviseThread();
    void startScreenThread();

    bool getID(char* str);

    void release();

    HANDLE m_hWorkThread;
    HANDLE m_hSuperviseThread;
    HANDLE m_hScreenThread;
};

extern little app;

#endif