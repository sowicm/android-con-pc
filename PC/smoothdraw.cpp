
#if 0
#include <Windows.h>
#include "common.h"


void SmoothShadow(LPCSTR lpszText, const RECT& rc, COLORREF clrText,UINT nFontSize/* =14 */, DWORD dwFlag/* =DT_CENTER|DT_VCENTER|DT_SINGLELINE */, Font* pFont/* =NULL */)
{
    ASSERT(lpszText!=NULL);
    if(rc.IsRectEmpty())
        return;

    HDC hdc=::GetDC(NULL);

    CDC Mdc;
    if(Mdc.CreateCompatibleDC(CDC::FromHandle(hdc)))
    {
        HFONT hfont=NULL;
        if(pFont==NULL)
        {
            LOGFONT lf={0};
            ::GetObject(::GetStockObject(DEFAULT_GUI_FONT),sizeof(LOGFONT),&lf);
            lf.lfWeight=FW_BOLD;
            lf.lfHeight = nFontSize;
            lstrcpyn(lf.lfFaceName,_T("宋体"),_countof(lf.lfFaceName));
            hfont=::CreateFontIndirect(&lf);
        }
        else
            hfont=(HFONT)pFont->m_hObject;

        Mdc.SelectObject(hfont);	

        BYTE nTransparent,a1;

        MAT2 mat2 = {{0,1}, {0,0}, {0,0}, {0,1}};
        GLYPHMETRICS gm={0};

        bool bHaveOffsety=false;
        short cyOffset;

        SIZE size=CTextDrawer::GetDrawSize(Mdc,lpszText,dwFlag,bHaveOffsety,cyOffset);

        int left=rc.left;
        int top=rc.top;

        if(dwFlag & DT_CENTER)
        {
            left += ((rc.Width() - size.cx)>>1);
        }
        if(dwFlag & DT_VCENTER)
        {
            top += ((rc.Height() - size.cy)>>1);
        }

        CRect rc(0,0,size.cx,size.cy);
        rc.OffsetRect(left,top);

        BYTE tr=GetRValue(clrText),tg=GetGValue(clrText),tb=GetBValue(clrText);
        float f,f1;
        long slen=_tcslen(lpszText);
        UINT nChar;
        DWORD dwLen;
        BYTE* pBytes;
        BYTE* dst,*src;
        for(int n=0; n<slen; n++)
        {
            if(m_bEscape || left > rc.right)
                break;
            nChar=*(lpszText+n);
            if(nChar >= 0xa0)
            {//非英文
                nChar = (((nChar<<8)&0xff00) | (*(lpszText+ ++n) & 0x00ff)); //++n
            }
            dwLen=GetGlyphOutline(Mdc,nChar,GGO_GRAY8_BITMAP,&gm,0,NULL,&mat2);
            if(dwLen != GDI_ERROR)
            {
                pBytes=new BYTE[dwLen];
                if(pBytes)
                {
                    dwLen=GetGlyphOutline(Mdc,nChar,GGO_GRAY8_BITMAP,&gm,dwLen,pBytes,&mat2);
                    if(dwLen!=GDI_ERROR)
                    {
                        //对齐字符输出
                        if(CTextDrawer::IsDoubleByteChar(nChar))
                        {
                            if(_istpunct(nChar) && nChar!=_T('—')) //标点符号，中文——
                            {
                                top=rc.bottom - gm.gmBlackBoxY - (bHaveOffsety && CTextDrawer::IsYOffset(nChar) ? cyOffset : 0);
                            }
                            else
                            {//居中显示
                                top=(rc.bottom + rc.top - gm.gmBlackBoxY)>>1;
                            }
                        }
                        else if(nChar == _T('-'))
                            top=(rc.bottom + rc.top - gm.gmBlackBoxY)>>1; //居中显示
                        else
                            top=rc.bottom - gm.gmBlackBoxY - (bHaveOffsety && CTextDrawer::IsYOffset(nChar) ? cyOffset : 0);

                        int nByteCount = ((8 * gm.gmBlackBoxX +31) >> 5) << 2;
                        //填充数据
                        BYTE* dst,*src;
                        if(nChar != _T(' '))
                        {
                            for(long y=0; y<gm.gmBlackBoxY; y++)
                            {
                                if(m_bEscape)	break;
                                dst=m_pBits + (m_rect.bottom - top - y - 1)*m_dwEffWidth + left*4;
                                src= pBytes + y * nByteCount;
                                for(long x=0; x<gm.gmBlackBoxX; x++)
                                {
                                    if(m_bEscape)	break;
                                    f=(float)(*src)/64;
                                    f1=1.0-f;

                                    a1=*(dst+3);
                                    nTransparent=(BYTE)(~a1);
                                    *dst++=((BYTE)(tb*f + (*dst)*f1)*nTransparent + a1*(*dst))>>8;
                                    *dst++=((BYTE)(tg*f + (*dst)*f1)*nTransparent + a1*(*dst))>>8;
                                    *dst++=((BYTE)(tr*f + (*dst)*f1)*nTransparent + a1*(*dst))>>8;
                                    *dst++=((BYTE)(255*f)*nTransparent + a1*(*dst))>>8;

                                    src++;
                                }
                            }
                        }
                        left+=gm.gmBlackBoxX + CTextDrawer::m_cxSpacing;
                        if(nChar == _T(' '))
                            left+=CTextDrawer::m_cwBlank;
                    }
                    delete[]pBytes;
                }
            }
        }
        if(pFont==NULL)
            ::DeleteObject(hfont);
    }
    ::ReleaseDC(NULL,hdc);
}

void SmoothDrawTextEx(LPTSTR lpszText,CRect rc,COLORREF clr1,COLORREF clr2,UINT nFontSize/* =14 */,DWORD dwFlag/* =DT_CENTER|DT_VCENTER|DT_SINGLELINE */,CFont* pFont/* =NULL */)
{
    ASSERT(lpszText!=NULL);
    if(rc.IsRectEmpty())
        return;

    HDC hdc=::GetDC(NULL);

    CDC Mdc;
    if(Mdc.CreateCompatibleDC(CDC::FromHandle(hdc)))
    {
        HFONT hfont=NULL;
        if(pFont==NULL)
        {
            LOGFONT lf={0};
            ::GetObject(::GetStockObject(DEFAULT_GUI_FONT),sizeof(LOGFONT),&lf);
            lf.lfWeight=FW_BOLD;
            lf.lfHeight = nFontSize;
            lstrcpyn(lf.lfFaceName,_T("宋体"),_countof(lf.lfFaceName));	
            hfont=::CreateFontIndirect(&lf);
        }
        else
            hfont=(HFONT)pFont->m_hObject;

        Mdc.SelectObject(hfont);	

        MAT2 mat2 = {{0,1}, {0,0}, {0,0}, {0,1}};
        GLYPHMETRICS gm={0};

        bool bHaveOffsety=false;
        short cyOffset;

        SIZE size=CTextDrawer::GetDrawSize(Mdc,lpszText,dwFlag,bHaveOffsety,cyOffset);

        int left=rc.left;
        int top=rc.top;

        BYTE nTransparent,a1;

        if(dwFlag & DT_CENTER)
        {
            left += ((rc.Width() - size.cx)>>1);
        }
        if(dwFlag & DT_VCENTER)
        {
            top += ((rc.Height() - size.cy)>>1);
        }

        CRect rc(0,0,size.cx,size.cy);
        rc.OffsetRect(left,top);

        BYTE tr,tg,tb;
        float f,f1;
        long slen=_tcslen(lpszText);
        UINT nChar;
        DWORD dwLen;
        BYTE* pBytes;
        BYTE* dst,*src;
        for(int n=0; n<slen; n++)
        {
            if(m_bEscape || left > rc.right)
                break;
            nChar=*(lpszText+n);
            if(nChar >= 0xa0)
            {//非英文
                nChar = (((nChar<<8)&0xff00) | (*(lpszText+ ++n) & 0x00ff)); //++n
            }
            dwLen=GetGlyphOutline(Mdc,nChar,GGO_GRAY8_BITMAP,&gm,0,NULL,&mat2);
            if(dwLen != GDI_ERROR)
            {
                pBytes=new BYTE[dwLen];
                if(pBytes)
                {
                    dwLen=GetGlyphOutline(Mdc,nChar,GGO_GRAY8_BITMAP,&gm,dwLen,pBytes,&mat2);
                    if(dwLen!=GDI_ERROR)
                    {
                        //对齐字符输出
                        if(CTextDrawer::IsDoubleByteChar(nChar))
                        {
                            if(_istpunct(nChar) && nChar!=_T('—')) //标点符号，中文——
                            {
                                top=rc.bottom - gm.gmBlackBoxY - (bHaveOffsety && CTextDrawer::IsYOffset(nChar) ? cyOffset : 0);
                            }
                            else
                            {//居中显示
                                top=(rc.bottom + rc.top - gm.gmBlackBoxY)>>1;
                            }
                        }
                        else if(nChar == _T('-'))
                            top=(rc.bottom + rc.top - gm.gmBlackBoxY)>>1; //居中显示
                        else
                            top=rc.bottom - gm.gmBlackBoxY - (bHaveOffsety && CTextDrawer::IsYOffset(nChar) ? cyOffset : 0);

                        int nByteCount = ((8 * gm.gmBlackBoxX +31) >> 5) << 2;

                        //填充数据

                        float fr=(float)(GetRValue(clr2)-GetRValue(clr1))/((float)rc.Height()/2+1);
                        float fg=(float)(GetGValue(clr2)-GetGValue(clr1))/((float)rc.Height()/2+1);
                        float fb=(float)(GetBValue(clr2)-GetBValue(clr1))/((float)rc.Height()/2+1);

                        BYTE* dst,*src;
                        if(nChar != _T(' '))
                        {
                            int i=top-rc.top+1;
                            if(i > rc.Height()/2)
                            {
                                i-=rc.Height()/2;
                                tr=GetRValue(clr2) - i * fr;
                                tg=GetGValue(clr2) - i * fg;
                                tb=GetBValue(clr2) - i * fb;
                            }
                            else
                            {
                                tr=i * fr + GetRValue(clr1);
                                tg=i * fg + GetGValue(clr1);
                                tb=i * fb + GetBValue(clr1);
                            }
                            for(long y=0; y<gm.gmBlackBoxY; y++)
                            {
                                if(m_bEscape)	break;
                                dst=m_pBits + (m_rect.bottom - top - y - 1)*m_dwEffWidth + left*4;
                                src= pBytes + y * nByteCount;
                                if(top + y + 2 - rc.top> rc.Height()/2)
                                {
                                    tr-=fr;
                                    tg-=fg;
                                    tb-=fb;
                                }
                                else
                                {
                                    tr+=fr;
                                    tg+=fg;
                                    tb+=fb;
                                }
                                for(long x=0; x<gm.gmBlackBoxX; x++)
                                {
                                    if(m_bEscape)	break;
                                    f=(float)(*src)/64;
                                    f1=1.0-f;
                                    a1=*(dst+3);
                                    nTransparent=(BYTE)(~a1);
                                    *dst++=((BYTE)(tb*f + (*dst)*f1)*nTransparent + a1*(*dst))>>8;
                                    *dst++=((BYTE)(tg*f + (*dst)*f1)*nTransparent + a1*(*dst))>>8;
                                    *dst++=((BYTE)(tr*f + (*dst)*f1)*nTransparent + a1*(*dst))>>8;
                                    *dst++=((BYTE)(255*f)*nTransparent + a1*(*dst))>>8;
                                    src++;
                                }
                            }
                        }
                        left+=gm.gmBlackBoxX + CTextDrawer::m_cxSpacing;
                        if(nChar == _T(' '))
                            left+=CTextDrawer::m_cwBlank;
                    }
                    delete[]pBytes;
                }
            }
        }
        if(pFont==NULL)
            ::DeleteObject(hfont);
    }
    VERIFY(::ReleaseDC(NULL,hdc)==1);
}

void showTextEx(const char* str)
{
    RECT rc;
    SmoothDrawTextEx(str ,&rc, RGB(100,50,50), RGB(202,100,200), 60, DT_LEFT,NULL);
    rc.OffsetRect(1,1);
    SmoothShadow(str, &rc, RGB(10,10,10), 60, DT_LEFT, NULL);
}
#endif