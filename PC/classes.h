
#ifndef classes_h
#define classes_h

#include <set>
using std::set;

#include <SRect.h>


int lasttime = 0;
char strLastModified[128] = "";
char strHeader[64] = "";
struct addrinfo;
bool      dark = false;

class RenderObject;
set<RenderObject*> g_renderObjects;

class GIFObject;
GIFObject* g_gif1;

HRESULT __stdcall CreateStreamFromResource(HMODULE hModule, LPCWSTR lpName, LPSTREAM *ppStream)
{
    HRSRC hResInfo = FindResourceW(NULL, lpName, RT_RCDATA);
    DWORD dwSize = SizeofResource(NULL, hResInfo);
    HGLOBAL hResData = LoadResource(NULL, hResInfo);
    LPVOID pResLock = LockResource(hResData);
    HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE | GMEM_NODISCARD, dwSize);
    LPVOID pBuffer = GlobalLock(hGlobal);
    memcpy(pBuffer, pResLock, dwSize);
    GlobalUnlock(hGlobal);
    
    HRESULT hr = CreateStreamOnHGlobal(hGlobal, TRUE, ppStream);

    FreeResource(hResData);

    return hr;
}

class RenderObject
{
public:
    RenderObject()
    {
        m_pImage = nullptr;
    }
    RenderObject(HMODULE hModule, int IDR, int x, int y)
    {
        IStream *pStream;
        CreateStreamFromResource(hModule, MAKEINTRESOURCE(IDR), &pStream);
        init(new Image(pStream), x, y);
    }
    virtual ~RenderObject()
    {
        if (m_pImage)
            delete m_pImage;
    }

    virtual void render(Graphics& painter)
    {
        painter.DrawImage(m_pImage, m_rect.x, m_rect.y, m_rect.width, m_rect.height);
    }
    void init(Image* image, int x, int y)
    {
        m_pImage = image;
        m_rect.x = x;
        m_rect.y = y;
        m_rect.width = m_pImage->GetWidth();
        m_rect.height = m_pImage->GetHeight();
    }

    Image* m_pImage;
    SRect  m_rect;
};

class AnimationObject : public RenderObject
{
public:
    AnimationObject()
    {
    }
    AnimationObject(HMODULE hModule, int IDR, int x, int y, int row, int col)
    {
        IStream *pStream;
        CreateStreamFromResource(hModule, MAKEINTRESOURCE(IDR), &pStream);
        init(new Image(pStream), x, y, row, col);
    }

    virtual void render(Graphics& painter)
    {
        painter.DrawImage(m_pImage, m_rect.x, m_rect.y, m_srcRect.x, m_srcRect.y, m_srcRect.width, m_srcRect.height, UnitPixel);
    }
    void init(Image* image, int x, int y, int row, int col)
    {
        m_pImage = image;
        m_rect.x = x;
        m_rect.y = y;
        m_rect.width = m_pImage->GetWidth();
        m_rect.height = m_pImage->GetHeight();
        m_srcRect.width = m_rect.width / col;
        m_srcRect.height = m_rect.height / row;
        m_nRow = row;
        m_nCol = col;
        m_nFrameCount = row * col;
        m_iFrame = 0;
        _frame();
    }
    void next()
    {
        ++m_iFrame;
        if (m_iFrame >= m_nFrameCount)
            m_iFrame = 0;
        _frame();
    }
    virtual void _frame()
    {
        m_srcRect.x = (m_iFrame % m_nCol) * m_srcRect.width;
        m_srcRect.y = (m_iFrame / m_nCol) * m_srcRect.height;
    }

    int m_iFrame, m_nFrameCount;
    int m_nRow, m_nCol;
    SRect m_srcRect;
};

class GIFObject : public AnimationObject
{
public:
    GIFObject(HMODULE hModule, int IDR, int x, int y)
    {
        IStream *pStream;
        CreateStreamFromResource(hModule, MAKEINTRESOURCE(IDR), &pStream);
        m_pImage = new Image(pStream);
        int count = m_pImage->GetFrameDimensionsCount();
        GUID *pDimensionIDs = new GUID[count];
        m_pImage->GetFrameDimensionsList(pDimensionIDs, count);
        m_nFrameCount = m_pImage->GetFrameCount(pDimensionIDs);
        delete[] pDimensionIDs;

        m_iFrame = 0;
        m_rect.width = m_pImage->GetWidth();
        m_rect.height = m_pImage->GetHeight();

        m_rect.x = x;
        m_rect.y = y;

    }

    virtual void render(Graphics& painter)
    {
        painter.DrawImage(m_pImage, m_rect.x, m_rect.y, m_pImage->GetWidth(), m_pImage->GetHeight());
    }
    virtual void _frame()
    {
        m_pImage->SelectActiveFrame(&FrameDimensionTime, m_iFrame);
    }
};

/*
class Snow
{
public:
    static Image* m_pImage;
    static SSize m_imgSize;

    Snow(){}

    void render(Graphics& painter)
    {
        
    }

    static void setImage(HMODULE hModule, int IDR)
    {
        IStream *pStream;
        CreateStreamFromResource(hModule, MAKEINTRESOURCE(IDR), &pStream);
        m_pImage = new Image(pStream);
        m_imgSize.width = m_pImage->GetWidth();
        m_imgSize.height = m_pImage->GetHeight();

        int count = m_pImage->GetFrameDimensionsCount();
        GUID *pDimensionIDs = new GUID[count];
        m_pImage->GetFrameDimensionsList(pDimensionIDs, count);

    }

    void init(int x, int size)
    {

    }

};
*/

#endif