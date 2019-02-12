// HexEdit.cpp : implementation file
//

#include "stdafx.h"
#include "HexEdit.h"


// CHexEdit

IMPLEMENT_DYNAMIC(CHexEdit, CEdit)

CHexEdit::CHexEdit()
{
}

CHexEdit::~CHexEdit()
{
}


BEGIN_MESSAGE_MAP(CHexEdit, CEdit)
    ON_WM_KEYDOWN()
    ON_WM_CHAR()
    ON_WM_LBUTTONDOWN()
    ON_WM_LBUTTONUP()
    ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()



// CHexEdit message handlers
void CHexEdit::OnKeyDown( UINT nChar, UINT nRepCnt, UINT nFlags )
{
    int nStartChar, nEndChar;

    switch (nChar) {
    case VK_LEFT:
        CEdit::OnKeyDown(nChar,nRepCnt,nFlags);
        GetSel(nStartChar,nEndChar);
        if ( nStartChar % 3 != 0 )
        {
            nStartChar = nStartChar - (nStartChar%3);
            SetSel(nStartChar,nStartChar);
        }
        break;
    case VK_RIGHT:
        CEdit::OnKeyDown(nChar,nRepCnt,nFlags);
        GetSel(nStartChar,nEndChar);
        if ( nStartChar % 3 != 0 )
        {
            nStartChar = nStartChar - (nStartChar%3) + 3;
            SetSel(nStartChar,nStartChar);
        }
        break;
    case VK_UP:
    case VK_DOWN:
        CEdit::OnKeyDown(nChar,nRepCnt,nFlags);
        GetSel(nStartChar,nEndChar);
        if ( nStartChar % 3 != 0 )
        {
            nStartChar -= (nStartChar%3);
            SetSel(nStartChar,nStartChar);
        }
        break;
    case VK_DELETE:
        GetSel(nStartChar,nEndChar);
        if ( nStartChar == nEndChar )
        {
            nStartChar -= (nStartChar%3);
            nEndChar = nEndChar - (nEndChar%3) + 3;
        }
        SetSel(nStartChar,nEndChar);
        CEdit::OnKeyDown(nChar,nRepCnt,nFlags);
        break;
    }
}

void CHexEdit::OnChar( UINT nChar, UINT nRepCnt, UINT nFlags )
{
    // ES_LOWERCASE
    // ES_UPPERCASE
    int nStartChar, nEndChar;

    if ( isxdigit(nChar) )
    {
        GetSel(nStartChar,nEndChar);
        int n = (nStartChar % 3);
        switch(n) {
            case 0:
                CEdit::OnChar(nChar, nRepCnt, nFlags);
                ReplaceSel( _T("0 ") );
                SetSel(nStartChar+1,nStartChar+1);
                break;

            case 1:
                SetSel(nStartChar,nStartChar+1);
                CEdit::OnChar(nChar, nRepCnt, nFlags);
                SetSel(nStartChar+2,nStartChar+2);
                break;
        }
    }
}

void CHexEdit::OnLButtonDown( UINT nFlags, CPoint point )
{
    int nStartChar, nEndChar;

    CEdit::OnLButtonDown(nFlags, point);

    GetSel(nStartChar,nEndChar);
    if ( nStartChar % 3 != 0 )
    {
        nStartChar -= (nStartChar%3);
        SetSel(nStartChar,nEndChar);
    }
}

void CHexEdit::OnLButtonUp( UINT nFlags, CPoint point )
{
    int nStartChar, nEndChar;

    CEdit::OnLButtonUp(nFlags, point);

    GetSel(nStartChar,nEndChar);
    if ( nStartChar % 3 )
        nStartChar -= (nStartChar%3);
    if ( nEndChar % 3)
        nEndChar = nEndChar - (nEndChar%3) + 3;
    SetSel(nStartChar,nEndChar);
}

void CHexEdit::OnMouseMove( UINT nFlags, CPoint point )
{
    CEdit::OnMouseMove(nFlags, point);
}
