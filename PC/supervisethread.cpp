
#include <Windows.h>
#include <time.h>
#include <process.h>

#include "common.h"
#include "little.h"

int nWorkThreadWorkClock = 0;

void supervisethreadStart(void *lpThreadParameter)
{
    while (true)
    {
        Sleep(2000);
        if (nWorkThreadWorkClock + 30000 < clock())
        {
            if (sck != INVALID_SOCKET)
            {
                closesocket(sck);
                sck = INVALID_SOCKET;
            }
            bWorkThreadRun = false;
            if (app.m_hWorkThread != nullptr)
            {
                WaitForSingleObject(app.m_hWorkThread, INFINITE);
                CloseHandle(app.m_hWorkThread);
            }
            app.startWorkThread();
        }
    }

    _endthread();
}