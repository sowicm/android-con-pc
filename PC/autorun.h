
#include <Shlobj.h>
#include <Shobjidl.h>
#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")

void setAutoRun()
{    
    //{ BEGIN ShellLink

    LPITEMIDLIST pidl;
    wchar_t path[MAX_PATH + 1];

    SHGetSpecialFolderLocation(NULL, CSIDL_STARTUP, &pidl);

    SHGetPathFromIDList(pidl, path);

    wcscat(path, L"\\little.lnk");

    wchar_t AppPath[MAX_PATH + 1];
    GetModuleFileName(NULL, AppPath, MAX_PATH + 1);

    HRESULT hr;
    IShellLink *psl;
    IPersistFile *pPf;

    CoInitialize(NULL);

    hr = CoCreateInstance(CLSID_ShellLink,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_IShellLink,
        (LPVOID*)&psl);
    //if (FAILED(hr))

    hr = psl->QueryInterface(IID_IPersistFile, (LPVOID*)&pPf);

    if (PathFileExists(path))
    {
        hr = pPf->Load(path, 0);
        wchar_t target[MAX_PATH + 1];
        psl->GetPath(target, MAX_PATH + 1, NULL, SLGP_UNCPRIORITY);
        if (wcscmp(target, AppPath) != 0)
        {
            psl->SetPath(AppPath);
            pPf->Save(path, TRUE);
        }
    }
    else
    {
        hr = psl->SetPath(AppPath);
        hr = pPf->Save(path, TRUE);
    }


    pPf->Release();
    psl->Release();

    CoUninitialize();

    //} ShellLink END
}