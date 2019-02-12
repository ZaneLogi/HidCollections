// EditListCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "EditListCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#pragma comment(lib,"msimg32.lib")

int CALLBACK CEditListCtrl::CompareFunction( LPARAM lParam1, LPARAM lParam2, LPARAM lParamData )
{
    UNREFERENCED_PARAMETER(lParam1);
    UNREFERENCED_PARAMETER(lParam2);

    CEditListCtrl* pListCtrl = reinterpret_cast<CEditListCtrl*>( lParamData );
    ASSERT( pListCtrl->IsKindOf( RUNTIME_CLASS( CListCtrl ) ) );

    // lParam1 and lParam2 are the item data of the items being compared

    return 0;
}

/////////////////////////////////////////////////////////////////////////////
// CInPlaceEditBox

CInPlaceEditBox::CInPlaceEditBox(int iItem, int iSubItem, CString sInitText)
:m_sInitText( sInitText )
{
    m_iItem = iItem;
    m_iSubItem = iSubItem;
    m_bESC = FALSE;
}

CInPlaceEditBox::~CInPlaceEditBox()
{
}


BEGIN_MESSAGE_MAP(CInPlaceEditBox, CEdit)
    //{{AFX_MSG_MAP(CInPlaceEditBox)
    ON_WM_KILLFOCUS()
    ON_WM_NCDESTROY()
    ON_WM_CHAR()
    ON_WM_CREATE()
    ON_WM_KEYDOWN()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CInPlaceEditBox message handlers

BOOL CInPlaceEditBox::PreTranslateMessage(MSG* pMsg)
{
    if( pMsg->message == WM_KEYDOWN )
    {
        if( pMsg->wParam == VK_RETURN ||
            pMsg->wParam == VK_DELETE ||
            pMsg->wParam == VK_ESCAPE ||
            GetKeyState( VK_CONTROL) )
        {
            ::TranslateMessage(pMsg);
            ::DispatchMessage(pMsg);
            return TRUE;                // DO NOT process further
        }
    }

    return CEdit::PreTranslateMessage(pMsg);
}


void CInPlaceEditBox::OnKillFocus(CWnd* pNewWnd)
{
    CEdit::OnKillFocus(pNewWnd);

    CString str;
    GetWindowText(str);

    // Send Notification to parent of ListView ctrl
    LV_DISPINFO dispinfo;
    dispinfo.hdr.hwndFrom = GetParent()->m_hWnd;
    dispinfo.hdr.idFrom = GetDlgCtrlID();
    dispinfo.hdr.code = LVN_ENDLABELEDIT;

    dispinfo.item.mask = LVIF_TEXT;
    dispinfo.item.iItem = m_iItem;
    dispinfo.item.iSubItem = m_iSubItem;
    dispinfo.item.pszText = m_bESC ? NULL : LPTSTR((LPCTSTR)str);
    dispinfo.item.cchTextMax = str.GetLength();
    dispinfo.item.lParam = 0;

    GetParent()->GetParent()->SendMessage( WM_NOTIFY, GetParent()->GetDlgCtrlID(), 
                    (LPARAM)&dispinfo );

    DestroyWindow();
}

void CInPlaceEditBox::OnNcDestroy()
{
    CEdit::OnNcDestroy();

    delete this;
}

void CInPlaceEditBox::ResizeToFit()
{
    // Get text extent
    CString str;

    GetWindowText( str );
    CWindowDC dc(this);
    CFont *pFont = GetParent()->GetFont();
    CFont *pFontDC = dc.SelectObject( pFont );
    CSize size = dc.GetTextExtent( str );
    dc.SelectObject( pFontDC );
    size.cx += 5;                   // add some extra buffer

    // Get client rect
    CRect rect, parentrect;
    GetClientRect( &rect );
    GetParent()->GetClientRect( &parentrect );

    // Transform rect to parent coordinates
    ClientToScreen( &rect );
    GetParent()->ScreenToClient( &rect );

    // Check whether control needs to be resized
    // and whether there is space to grow
    if( size.cx > rect.Width() )
    {
        if( size.cx + rect.left < parentrect.right )
            rect.right = rect.left + size.cx;
        else
            rect.right = parentrect.right;
        MoveWindow( &rect );
    }
}

void CInPlaceEditBox::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    if( nChar == VK_ESCAPE || nChar == VK_RETURN)
    {
        if( nChar == VK_ESCAPE )
            m_bESC = TRUE;
        GetParent()->SetFocus();
        return;
    }

    CEdit::OnChar(nChar, nRepCnt, nFlags);

    // Resize edit control if needed
    ResizeToFit();
}

int CInPlaceEditBox::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CEdit::OnCreate(lpCreateStruct) == -1)
        return -1;

    // Set the proper font
    CFont* font = GetParent()->GetFont();
    SetFont(font);

    SetWindowText( m_sInitText );
    SetFocus();
    SetSel( 0, -1 );
    return 0;
}

void CInPlaceEditBox::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
    CEdit::OnKeyDown(nChar, nRepCnt, nFlags);
}

/////////////////////////////////////////////////////////////////////////////
// CInPlaceHexEditBox

CInPlaceHexEditBox::CInPlaceHexEditBox( int count, int iItem, int iSubItem, CString sInitText)
    : CInPlaceEditBox(iItem,iSubItem,sInitText), m_nDigitCount(count)
{
}

CInPlaceHexEditBox::~CInPlaceHexEditBox()
{
}


BEGIN_MESSAGE_MAP(CInPlaceHexEditBox, CInPlaceEditBox)
    //{{AFX_MSG_MAP(CInPlaceHexEditBox)
    ON_WM_CHAR()
    ON_WM_CREATE()
    ON_WM_KEYDOWN()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CInPlaceHexEditBox message handlers

int CInPlaceHexEditBox::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
    if (CInPlaceEditBox::OnCreate(lpCreateStruct) == -1)
        return -1;
    
    if ( m_sInitText.GetLength() > m_nDigitCount ) {
        m_sInitText = m_sInitText.Left(m_nDigitCount);
    }
    else if ( m_sInitText.GetLength() < m_nDigitCount ) {
        CString s;
        LPTSTR pstr = s.GetBuffer(m_nDigitCount);
        int n = m_nDigitCount - m_sInitText.GetLength();
        int i;
        for ( i = 0; i < n; i++ ) pstr[i] = '0';
        for ( ; i < m_nDigitCount; i++ ) pstr[i] = m_sInitText[i-n];
        s.ReleaseBuffer();
        m_sInitText = s;
    }
    
    m_sInitText.MakeUpper();
    SetWindowText( m_sInitText );
    SetFocus();
    SetSel( 0, 0 );
    SetLimitText(m_nDigitCount);
    return 0;

}

void CInPlaceHexEditBox::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
    UNREFERENCED_PARAMETER(nFlags);
    UNREFERENCED_PARAMETER(nRepCnt);

    if( nChar == VK_ESCAPE || nChar == VK_RETURN)
    {
        if( nChar == VK_ESCAPE )
            m_bESC = TRUE;
        GetParent()->SetFocus();
        return;
    }

    int s,e;
    GetSel(s,e);
    if ( ('0' <= nChar && nChar <= '9')
        || ('A' <= nChar && nChar <= 'F')
        || ('a' <= nChar && nChar <= 'f') )
    {
        CString strText((char)nChar);
        strText.MakeUpper();

        if ( s == m_nDigitCount )
            return;

        if ( s == e ) e = s + 1;
        for ( ; s < e; s++ ) {
            SetSel(s,s+1);
            ReplaceSel(strText);
        }
        
        ResizeToFit();
    }
    else if ( nChar == VK_BACK ) {
        if ( e == 0 )
            return;

        if ( s == e ) s = e - 1;
        for ( ; e > s; e-- ) {
            SetSel(e-1,e);
            ReplaceSel(_T("0"));
        }
        SetSel(e,e);

        ResizeToFit();
    }
}

void CInPlaceHexEditBox::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
    int s,e;
    GetSel(s,e);
    if ( nChar == VK_DELETE ) {
        if ( s == m_nDigitCount )
            return;

        if ( s == e ) e = s + 1;
        for ( ; s < e; s++ ) {
            SetSel(s,s+1);
            ReplaceSel(_T("0"));
        }
    }
    else
        CInPlaceEditBox::OnKeyDown(nChar, nRepCnt, nFlags);
}

/////////////////////////////////////////////////////////////////////////////
// CInPlaceComboBox

CInPlaceComboBox::CInPlaceComboBox(int iItem, int iSubItem, CString sInitText, const CStringArray& astrComboItem )
{
    m_iItem = iItem;
    m_iSubItem = iSubItem;
    m_nSel = -1;

    for ( int i = 0; i < astrComboItem.GetSize(); i++ ) {
        m_lstItems.AddTail( astrComboItem[i] );
        if ( m_lstItems.GetTail() == sInitText ) m_nSel = i;
    }
    m_bESC = FALSE;
}

CInPlaceComboBox::~CInPlaceComboBox()
{
}


BEGIN_MESSAGE_MAP(CInPlaceComboBox, CComboBox)
    //{{AFX_MSG_MAP(CInPlaceComboBox)
    ON_WM_CREATE()
    ON_WM_KILLFOCUS()
    ON_WM_CHAR()
    ON_WM_NCDESTROY()
    ON_CONTROL_REFLECT(CBN_CLOSEUP, OnCloseup)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CInPlaceComboBox message handlers

int CInPlaceComboBox::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
    if (CComboBox::OnCreate(lpCreateStruct) == -1)
        return -1;
    
    // Set the proper font
    CFont* font = GetParent()->GetFont();
    SetFont(font);

    for( POSITION pos = m_lstItems.GetHeadPosition(); pos != NULL; )
    {
        AddString( (LPCTSTR) (m_lstItems.GetNext( pos )) );
    }
    SetCurSel( m_nSel );
    SetFocus();
    return 0;
}

BOOL CInPlaceComboBox::PreTranslateMessage(MSG* pMsg) 
{
    if( pMsg->message == WM_KEYDOWN )
    {
        if( pMsg->wParam == VK_RETURN ||
            pMsg->wParam == VK_ESCAPE )
        {
            ::TranslateMessage(pMsg);
            ::DispatchMessage(pMsg);
            return TRUE;                // DO NOT process further
        }
    }
    
    return CComboBox::PreTranslateMessage(pMsg);
}

void CInPlaceComboBox::OnKillFocus(CWnd* pNewWnd) 
{
    CComboBox::OnKillFocus(pNewWnd);
    
    CString str;
    GetWindowText(str);

    // Send Notification to parent of ListView ctrl
    LV_DISPINFO dispinfo;
    dispinfo.hdr.hwndFrom = GetParent()->m_hWnd;
    dispinfo.hdr.idFrom = GetDlgCtrlID();
    dispinfo.hdr.code = LVN_ENDLABELEDIT;

    dispinfo.item.mask = LVIF_TEXT;
    dispinfo.item.iItem = m_iItem;
    dispinfo.item.iSubItem = m_iSubItem;
    dispinfo.item.pszText = m_bESC ? NULL : LPTSTR((LPCTSTR)str);
    dispinfo.item.cchTextMax = str.GetLength();
    dispinfo.item.lParam = GetCurSel();

    GetParent()->GetParent()->SendMessage( WM_NOTIFY, GetParent()->GetDlgCtrlID(), (LPARAM)&dispinfo );

    PostMessage( WM_CLOSE );
}

void CInPlaceComboBox::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
    if( nChar == VK_ESCAPE || nChar == VK_RETURN)
    {
        if( nChar == VK_ESCAPE )
            m_bESC = TRUE;
        GetParent()->SetFocus();
        return;
    }
    
    CComboBox::OnChar(nChar, nRepCnt, nFlags);
}

void CInPlaceComboBox::OnNcDestroy() 
{
    CComboBox::OnNcDestroy();
    
    delete this;
}

void CInPlaceComboBox::OnCloseup() 
{
    GetParent()->SetFocus();
}

/////////////////////////////////////////////////////////////////////////////
// CEditListCtrl

CEditListCtrl::CEditListCtrl()
    : m_iSortColumn(-1),
      m_bSortAscending(true),
      m_bDragging(false)
{
}

CEditListCtrl::~CEditListCtrl()
{
}

BEGIN_MESSAGE_MAP(CEditListCtrl, CListCtrl)
    //{{AFX_MSG_MAP(CEditListCtrl)
    ON_WM_HSCROLL()
    ON_WM_VSCROLL()
    ON_NOTIFY_REFLECT(LVN_BEGINLABELEDIT, OnBeginLabelEdit)
    ON_NOTIFY_REFLECT(LVN_ENDLABELEDIT, OnEndLabelEdit)
    ON_WM_LBUTTONDOWN()
    ON_WM_LBUTTONUP()
    ON_WM_MOUSEMOVE()
    ON_WM_DESTROY()
    ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, OnCustomDraw)
    ON_NOTIFY_REFLECT(LVN_COLUMNCLICK, OnColumnClick)
    ON_NOTIFY_REFLECT(LVN_BEGINDRAG, OnBeginDrag )
    ON_WM_KEYDOWN()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEditListCtrl message handlers

void CEditListCtrl::PreSubclassWindow()
{
    // the list control must have the report style.
    ASSERT( ( GetStyle() & LVS_TYPEMASK ) == LVS_REPORT );

    CListCtrl::PreSubclassWindow();
    VERIFY( m_hcSortedHeader.SubclassWindow( GetHeaderCtrl()->GetSafeHwnd() ) );
}

void CEditListCtrl::EnableEditLabels(bool bEnabled)
{
    if ( bEnabled )
        ModifyStyle(0,LVS_EDITLABELS);
    else
        ModifyStyle(LVS_EDITLABELS,0);
}

// HitTestEx    - Determine the row index and column index for a point
// Returns    - the row index or -1 if point is not over a row
// point    - point to be tested.
// col        - to hold the column index
int CEditListCtrl::HitTestEx(CPoint &point, int *col) const
{
    int colnum = 0;
    int row = HitTest( point, NULL );

    if( col ) *col = 0;

    // Make sure that the ListView is in LVS_REPORT
    if( (GetWindowLong(m_hWnd, GWL_STYLE) & LVS_TYPEMASK) != LVS_REPORT )
        return row;

    // Get the top and bottom row visible
    row = GetTopIndex();
    int bottom = row + GetCountPerPage();
    if( bottom > GetItemCount() )
        bottom = GetItemCount();

    // Get the number of columns
    CHeaderCtrl* pHeader = (CHeaderCtrl*)GetDlgItem(0);
    int nColumnCount = pHeader->GetItemCount();

    // Loop through the visible rows
    for( ;row <=bottom;row++)
    {
        // Get bounding rect of item and check whether point falls in it.
        CRect rect;
        GetItemRect( row, &rect, LVIR_BOUNDS );
        if( rect.PtInRect(point) )
        {
            // Now find the column
            for( colnum = 0; colnum < nColumnCount; colnum++ )
            {
                int colwidth = GetColumnWidth(colnum);
                if( point.x >= rect.left && point.x <= (rect.left + colwidth ) )
                {
                    if( col ) *col = colnum;
                    return row;
                }
                rect.left += colwidth;
            }
        }
    }
    return -1;
}

// EditSubLabel        - Start edit of a sub item label
// Returns        - Temporary pointer to the new edit control
// nItem        - The row index of the item to edit
// nCol            - The column of the sub item.
void CEditListCtrl::EditSubLabel(int nItem, int nCol)
{
    // The returned pointer should not be saved

    // Make sure that the item is visible
    if( !EnsureVisible( nItem, TRUE ) ) return;

    // Make sure that nCol is valid
    CHeaderCtrl* pHeader = (CHeaderCtrl*)GetDlgItem(0);
    int nColumnCount = pHeader->GetItemCount();
    if( nCol >= nColumnCount || GetColumnWidth(nCol) < 5 )
        return;

    // Get the column offset
    int offset = 0;
    for( int i = 0; i < nCol; i++ )
        offset += GetColumnWidth( i );

    CRect rect;
    GetItemRect( nItem, &rect, LVIR_BOUNDS );

    // Now scroll if we need to expose the column
    CRect rcClient;
    GetClientRect( &rcClient );
    if( offset + rect.left < 0 || offset + rect.left > rcClient.right )
    {
        CSize size;
        size.cx = offset + rect.left;
        size.cy = 0;
        Scroll( size );
        rect.left -= size.cx;
    }

    // Get Column alignment
    LV_COLUMN lvcol;
    lvcol.mask = LVCF_FMT;
    GetColumn( nCol, &lvcol );

    rect.left += offset+4;
    rect.right = rect.left + GetColumnWidth( nCol ) - 3 ;
    if( rect.right > rcClient.right) rect.right = rcClient.right;

    HDITEM hdi;
    hdi.mask = HDI_LPARAM;
    m_hcSortedHeader.GetItem( nCol, &hdi );
    COLUMNDATA* pcd = (COLUMNDATA*)hdi.lParam;

    switch ( pcd->ct ) {
    case CT_NONE:
        break;
    case CT_EDITBOX:
        {
            DWORD dwStyle ;
            if((lvcol.fmt&LVCFMT_JUSTIFYMASK) == LVCFMT_LEFT)
                dwStyle = ES_LEFT;
            else if((lvcol.fmt&LVCFMT_JUSTIFYMASK) == LVCFMT_RIGHT)
                dwStyle = ES_RIGHT;
            else dwStyle = ES_CENTER;
            dwStyle |= WS_BORDER|WS_CHILD|WS_VISIBLE|ES_AUTOHSCROLL;
            CEdit *pEdit = new CInPlaceEditBox(nItem, nCol, GetItemText( nItem, nCol ));
            pEdit->Create( dwStyle, rect, this, 101 );
        }
        break;
    case CT_HEXEDITBOX:
        {
            DWORD dwStyle ;
            if((lvcol.fmt&LVCFMT_JUSTIFYMASK) == LVCFMT_LEFT)
                dwStyle = ES_LEFT;
            else if((lvcol.fmt&LVCFMT_JUSTIFYMASK) == LVCFMT_RIGHT)
                dwStyle = ES_RIGHT;
            else dwStyle = ES_CENTER;
            dwStyle |= WS_BORDER|WS_CHILD|WS_VISIBLE|ES_AUTOHSCROLL;
            CEdit *pEdit = new CInPlaceHexEditBox( pcd->count, nItem, nCol, GetItemText( nItem, nCol ));
            pEdit->Create( dwStyle, rect, this, 101 );
        }
        break;
    case CT_COMBOBOX:
        {
            rect.bottom += (rect.bottom-rect.top)*25;
        
            DWORD dwStyle = WS_BORDER|WS_CHILD|WS_VISIBLE|WS_VSCROLL//|WS_HSCROLL
                |CBS_DROPDOWNLIST|CBS_DISABLENOSCROLL;
            CComboBox *pList = new CInPlaceComboBox(nItem, nCol, GetItemText( nItem, nCol ), pcd->astrComboItem );
            pList->Create( dwStyle, rect, this, 101 );
            pList->SetHorizontalExtent( GetColumnWidth( nCol ));
        }
        break;
    case CT_PROGRESSBAR:
        // nothing to do
        break;
    case CT_CHECKBOX:
        {
            CString str = GetItemText(nItem,nCol);
            SetItemText(nItem,nCol,str==_T("v")?_T(" "):_T("v")); // invert

            ItemChangedInfo ici;
            ici.iItem = nItem;
            ici.iSubItem = nCol;
            ici.iCheckState = (str=="v"? 0:1);
            GetParent()->SendMessage( UD_LC_ITEM_CHANGED, nItem, (LPARAM)&ici );
        }
        break;
    }
}

void CEditListCtrl::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
    if( GetFocus() != this ) SetFocus();
    CListCtrl::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CEditListCtrl::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
    if( GetFocus() != this ) SetFocus();
    CListCtrl::OnVScroll(nSBCode, nPos, pScrollBar);
}

void CEditListCtrl::OnBeginLabelEdit(NMHDR* pNMHDR, LRESULT* pResult)
{
    LV_DISPINFO* plvDispInfo = (LV_DISPINFO*)pNMHDR;
    LV_ITEM *plvItem = &plvDispInfo->item;
    if ( plvItem->iSubItem >= 0 ) *pResult = TRUE;
    else *pResult = 0; 
}

void CEditListCtrl::OnEndLabelEdit(NMHDR* pNMHDR, LRESULT* pResult) 
{
    LV_DISPINFO *plvDispInfo = (LV_DISPINFO *)pNMHDR;
    LV_ITEM    *plvItem = &plvDispInfo->item;

    bool bChanged = false;
    if ( plvItem->pszText != NULL &&
        GetItemText(plvItem->iItem, plvItem->iSubItem) != plvItem->pszText )
    {
        SetItemText(plvItem->iItem, plvItem->iSubItem, plvItem->pszText);
        bChanged = true;
    }
    *pResult = FALSE;

    if ( bChanged ) {
        ItemChangedInfo ici;
        ici.iItem = plvItem->iItem;
        ici.iSubItem = plvItem->iSubItem;
        ici.iComboSel = (int)plvItem->lParam;
        ici.pList = this;
        GetParent()->SendMessage( UD_LC_ITEM_CHANGED, plvItem->iItem, (LPARAM)&ici );
    }
}

void CEditListCtrl::OnLButtonDown(UINT nFlags, CPoint point) 
{
    CListCtrl::OnLButtonDown(nFlags, point);

    int index, colnum;
    if( ( index = HitTestEx( point, &colnum )) != -1 ) {
        UINT flag = LVIS_FOCUSED;
        if( (GetItemState( index, flag ) & flag) == flag && colnum > 0)    {
            // Add check for LVS_EDITLABELS
            if( GetWindowLong(m_hWnd, GWL_STYLE) & LVS_EDITLABELS )
                EditSubLabel( index, colnum );
        }
        else
            SetItemState( index, LVIS_SELECTED | LVIS_FOCUSED , LVIS_SELECTED | LVIS_FOCUSED);
    }
}

void CEditListCtrl::OnDestroy()
{
    for ( int i = 0; i < m_hcSortedHeader.GetItemCount(); i++ ) {
        HDITEM hdi;
        hdi.mask = HDI_LPARAM;
        m_hcSortedHeader.GetItem( i, &hdi );
        delete (COLUMNDATA*)hdi.lParam;
    }
    CListCtrl::OnDestroy();
}

void CEditListCtrl::OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult)
{
    LPNMLVCUSTOMDRAW  lplvcd = (LPNMLVCUSTOMDRAW)pNMHDR;
    *pResult = CDRF_DODEFAULT;

    switch(lplvcd->nmcd.dwDrawStage) {
    case CDDS_PREPAINT:
        *pResult = CDRF_NOTIFYITEMDRAW;
        break;
    case CDDS_ITEMPREPAINT:
        *pResult = CDRF_NOTIFYSUBITEMDRAW|CDRF_NOTIFYPOSTPAINT;
        break;
    case CDDS_ITEMPREPAINT|CDDS_SUBITEM:
        {
            int iCol = lplvcd->iSubItem;
            int iRow = (int)lplvcd->nmcd.dwItemSpec;

            HDITEM hdi;
            hdi.mask = HDI_LPARAM;
            m_hcSortedHeader.GetItem( iCol, &hdi );
            COLUMNDATA* pcd = (COLUMNDATA*)hdi.lParam;

            CRect rcItem;
            GetSubItemRect( iRow, iCol, LVIR_BOUNDS, rcItem );
            CString strItem = GetItemText( iRow, iCol );

            bool bSelected = GetItemState( iRow, LVIS_SELECTED ) == LVIS_SELECTED;
            bool bDropHilited = GetItemState( iRow, LVIS_DROPHILITED ) == LVIS_DROPHILITED;
            bool bFocus = ( GetFocus() == this );
            bool bShowSelAlways = ( (GetStyle() & LVS_SHOWSELALWAYS) != 0 );

            switch ( pcd->ct ) {
            case CT_PROGRESSBAR:
                {
                    CDC* pDC = CDC::FromHandle(lplvcd->nmcd.hdc);
                    pDC->FillSolidRect( rcItem, ::GetSysColor(COLOR_BTNFACE) );

                    if ( bSelected ) {
                        if ( bShowSelAlways )
                            pDC->FillSolidRect( rcItem, ::GetSysColor( bFocus ? COLOR_HIGHLIGHT : COLOR_INACTIVEBORDER ) );
                        else
                            pDC->FillSolidRect( rcItem, bFocus ? ::GetSysColor(COLOR_HIGHLIGHT) : ::GetSysColor(COLOR_BTNFACE) );
                    }
                    else if ( bDropHilited ) {
                        pDC->FillSolidRect( rcItem, ::GetSysColor( COLOR_HIGHLIGHT ) );
                    }
                    else {
                        pDC->FillSolidRect( rcItem, ::GetSysColor(COLOR_BTNFACE) );
                    }

                    rcItem.DeflateRect(1,1);
                    pDC->SelectStockObject(NULL_BRUSH);
                    pDC->SelectStockObject(BLACK_PEN);
                    pDC->Rectangle( rcItem );
                    rcItem.DeflateRect(1,1);

                    CRect rcLeft(rcItem), rcRight(rcItem);
                    rcLeft.right = rcLeft.left + MulDiv((int)_tstof(strItem), rcItem.Width(), 100);
                    rcRight.left = rcLeft.right;

                    if ( strItem == "N/A" ) {
                        pDC->DrawText( strItem, rcItem, DT_VCENTER|DT_CENTER|DT_SINGLELINE);
                    }
                    else {
                        int percentage = (int)_tstof(strItem);
                        int r1 = GetRValue(pcd->crPBColor1) << 8;
                        int g1 = GetGValue(pcd->crPBColor1) << 8;
                        int b1 = GetBValue(pcd->crPBColor1) << 8;
                        int r2 = GetRValue(pcd->crPBColor2) << 8;
                        int g2 = GetGValue(pcd->crPBColor2) << 8;
                        int b2 = GetBValue(pcd->crPBColor2) << 8;
                        r2 = r1 + MulDiv( (r2-r1), percentage, 100 );
                        g2 = g1 + MulDiv( (g2-g1), percentage, 100 );
                        b2 = b1 + MulDiv( (b2-b1), percentage, 100 );
                        TRIVERTEX tv[] = {
                            { rcLeft.left, rcLeft.top,     (USHORT)r1, (USHORT)g1, (USHORT)b1, (USHORT)0xff00 },
                            { rcLeft.right, rcLeft.bottom, (USHORT)r2, (USHORT)g2, (USHORT)b2, (USHORT)0xff00 }
                        };
                        GRADIENT_RECT gr[] = { {0,1} };

                        ::GradientFill( pDC->GetSafeHdc(), tv, 2, gr, 1, GRADIENT_FILL_RECT_H );

                        strItem += "%";
                        pDC->DrawText( strItem, rcItem, DT_VCENTER|DT_CENTER|DT_SINGLELINE);
                    }

                    *pResult = CDRF_SKIPDEFAULT;
                }
                break;
            case CT_CHECKBOX:
                {
                    CDC* pDC = CDC::FromHandle(lplvcd->nmcd.hdc);
                    if ( bSelected ) {
                        if ( bShowSelAlways )
                            pDC->FillSolidRect( rcItem, ::GetSysColor( bFocus ? COLOR_HIGHLIGHT : COLOR_3DLIGHT ) );
                        else
                            pDC->FillSolidRect( rcItem, bFocus ? ::GetSysColor(COLOR_HIGHLIGHT) : pcd->crBkColor );
                    }
                    else if ( bDropHilited ) {
                        pDC->FillSolidRect( rcItem, ::GetSysColor( COLOR_HIGHLIGHT ) );
                    }
                    else {
                        pDC->FillSolidRect( rcItem, pcd->crBkColor );
                    }

                    UINT uState = DFCS_BUTTONCHECK;
                    if ( strItem == "v" ) uState |= DFCS_CHECKED;
                    pDC->DrawFrameControl( rcItem, DFC_BUTTON, uState );
                    *pResult = CDRF_SKIPDEFAULT;
                }
                break;
            case CT_EDITBOX:
            case CT_HEXEDITBOX:
            case CT_COMBOBOX:
            case CT_NONE:
                {
                    lplvcd->clrText = pcd->crTextColor;
                    lplvcd->clrTextBk = pcd->crBkColor;
                    *pResult = CDRF_NEWFONT;

                    GetParent()->SendMessage( UD_LC_SUBITEM_DRAW, (WPARAM)lplvcd, (LPARAM)this );
                }
                break;
            }
        }
        break;
    case CDDS_ITEMPOSTPAINT:
        {
            LRESULT i = GetParent()->SendMessage( UD_LC_ITEM_POSTPAINT, (WPARAM)lplvcd, (LPARAM)this );

            *pResult = ( i!=0 ? CDRF_SKIPDEFAULT : CDRF_DODEFAULT );
        }
        break;
    default:
        *pResult = CDRF_DODEFAULT;
        break;
    }
}

void CEditListCtrl::OnColumnClick( NMHDR* pNMHDR, LRESULT* pResult )
{
    NM_LISTVIEW* pnmv = (NM_LISTVIEW*)pNMHDR;
    *pResult = 0;
    const int iColumn = pnmv->iSubItem;

    // if it's a second click on the same column then reverse the sort order,
    // otherwise sort the new column in ascending order.
    Sort( iColumn, iColumn == m_iSortColumn ? !m_bSortAscending : TRUE );

}

void CEditListCtrl::OnBeginDrag( NMHDR* pNMHDR, LRESULT* pResult )
{
    NM_LISTVIEW* pnmv = (NM_LISTVIEW*)pNMHDR;
    *pResult = 0;

    DWORD dwStyle = GetStyle();
    if ( !(dwStyle & LVS_REPORT) || !(dwStyle & LVS_EDITLABELS) )
        return;

    // don't drag if this column is editable
    int nCol;
    CPoint ptHitTest(pnmv->ptAction);
    int n = HitTestEx( ptHitTest, &nCol);
    HDITEM hdi;
    hdi.mask = HDI_LPARAM;
    m_hcSortedHeader.GetItem( nCol, &hdi );
    COLUMNDATA* pcd = (COLUMNDATA*)hdi.lParam;
    if ( pcd->ct != CT_NONE )
        return;

    // ask app if it allows to move
    if ( 1 != GetParent()->SendMessage(UD_LC_QUERY_ROW_MOVE,pnmv->iItem,(LPARAM)this) )
        return;    

    SetFocus(); // when in-place control exists, this will close it

    m_nDragIndex = pnmv->iItem;

    // Create a drag image
    POINT pt;

    m_pDragImage = CreateDragImage(m_nDragIndex,&pt);
    ASSERT(m_pDragImage);

    CPoint ptBeginDrag( pnmv->ptAction );

    CRect rcItem;
    GetItemRect( m_nDragIndex, &rcItem, LVIR_BOUNDS );

    ptBeginDrag -= rcItem.TopLeft();

    m_pDragImage->BeginDrag(0,ptBeginDrag);
    m_pDragImage->DragEnter(GetDesktopWindow(), pnmv->ptAction );

    // Set dragging flag and others
    m_bDragging = true;
    m_nDropIndex = -1;

    SetCapture();
}

void CEditListCtrl::OnMouseMove(UINT nFlags, CPoint point) 
{
    CListCtrl::OnMouseMove(nFlags,point);

    if ( m_bDragging ) {
        CPoint ptScreen(point);
        ClientToScreen(&ptScreen);

        // MOVE THE DRAG IMAGE
        m_pDragImage->DragMove(ptScreen);

        // TEMPORARILY UNLOCK WINDOW UPDATES
        m_pDragImage->DragShowNolock(FALSE);

        // SCROLL VIEW IF NECESSARY
        int iOverItem = HitTest(point);
        int iTopItem = GetTopIndex();
        int iBottomItem = iTopItem + GetCountPerPage() - 1;

        if ( iOverItem == iTopItem && iTopItem != 0 ) {
            EnsureVisible(iOverItem - 1, false);
            UpdateWindow();
        }
        else if ( iOverItem == iBottomItem && iBottomItem != (GetItemCount() - 1) ) {
            EnsureVisible(iOverItem + 1, false);
            UpdateWindow();
        }

        UINT uFlags;
        int nNewDropIndex = HitTest(point,&uFlags);

        if ( m_nDropIndex != -1 ) {
            SetItemState( m_nDropIndex, 0, LVIS_DROPHILITED);
            RedrawItems(m_nDropIndex,m_nDropIndex);
            UpdateWindow();
        }

        if ( nNewDropIndex != -1 ) {
            SetItemState( nNewDropIndex, LVIS_DROPHILITED, LVIS_DROPHILITED);
            RedrawItems(nNewDropIndex,nNewDropIndex);
            UpdateWindow();
        }

        m_nDropIndex = nNewDropIndex;        
        if ( m_nDropIndex < 0 )
            SetCursor(LoadCursor(NULL,IDC_NO));
        else
            SetCursor(LoadCursor(NULL,IDC_SIZEALL));

        // LOCK WINDOW UPDATES
        m_pDragImage->DragShowNolock(TRUE);
    }
}

void CEditListCtrl::OnLButtonUp(UINT nFlags, CPoint point) 
{
    CListCtrl::OnLButtonUp(nFlags,point);

    if ( m_bDragging ) {
        ReleaseCapture();
        m_bDragging = false;
        m_pDragImage->DragLeave(GetDesktopWindow());
        m_pDragImage->EndDrag();
        delete m_pDragImage;

        if ( m_nDropIndex != -1 ) {
            SetItemState( m_nDropIndex, 0, LVIS_DROPHILITED);
            RedrawItems( m_nDropIndex, m_nDropIndex);
            UpdateWindow();
        }

        if ( DropItemOnList() ) {
            RowMovedInfo rmi;
            rmi.iNewIndex = m_nDropIndex;
            rmi.iOldIndex = m_nDragIndex;
            GetParent()->SendMessage( UD_LC_ROW_MOVED, (WPARAM)&rmi, (LPARAM)this );
        }
    }
}

bool CEditListCtrl::DropItemOnList()
{
    if ( m_nDropIndex == m_nDragIndex || m_nDropIndex == -1 ) {
        return false;
    }

    int nInsertIndex = m_nDropIndex;
    if ( m_nDropIndex > m_nDragIndex ) {
        nInsertIndex++;
    }

    int nOldIndex = m_nDragIndex;

    // GET INFORMATION ON THE DRAGGED ITEM BY SETTING AN LV_ITEM STRUCTURE
    // AND THEN CALLING GetItem TO FILL IT IN
    TCHAR szLabel[256];
    LV_ITEM lvi;
    ZeroMemory(&lvi, sizeof(LV_ITEM));
    lvi.mask = LVIF_TEXT | LVIF_IMAGE;
    lvi.stateMask = LVIS_DROPHILITED | LVIS_FOCUSED | LVIS_SELECTED;
    lvi.pszText = szLabel;
    lvi.iItem = m_nDragIndex;
    lvi.cchTextMax = 255;
    GetItem(&lvi);

    int nCheckState = GetCheck(nOldIndex);

    // INSERT THE DROPPED ITEM
    lvi.iItem = nInsertIndex;
    InsertItem(&lvi);

    SetCheck( nInsertIndex, nCheckState );

    // FILL IN ALL OF THE COLUMNS
    int nColumnCount = m_hcSortedHeader.GetItemCount();
    lvi.mask = LVIF_TEXT;
    lvi.iItem = nInsertIndex;

    if ( nOldIndex > nInsertIndex )
        nOldIndex++;

    for ( int col = 1; col < nColumnCount; col++ ) {
        lstrcpy( lvi.pszText, (LPCTSTR)(GetItemText(nOldIndex,col)) );
        lvi.iSubItem = col;
        SetItem(&lvi);
    }

    //DELETE THE ITEM THAT WAS DRAGGED FROM ITS ORIGINAL LOCATION
    DeleteItem(nOldIndex);

    m_hcSortedHeader.RedrawWindow();
    return true;
}

void CEditListCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
    DWORD dwStyle = GetStyle();
    if ( !(dwStyle & LVS_REPORT) || !(dwStyle & LVS_EDITLABELS) )
    {
        CListCtrl::OnKeyDown(nChar,nRepCnt,nFlags) ;
        return;
    }

    SetFocus(); // when in-place control exists, this will close it

    switch (nChar) {
    case VK_INSERT:
        {
            int nSel = GetFirstSelection();
            if ( nSel < 0 ) nSel = GetItemCount();
            if ( 1 == GetParent()->SendMessage(UD_LC_QUERY_ROW_INSERT,nSel,(LPARAM)this) ) {
                InsertItem(nSel,_T(""));
                GetParent()->SendMessage(UD_LC_ROW_INSERTED,nSel,(LPARAM)this);
            }
        }
        break;
    case VK_DELETE:
        {
            int nSel = GetFirstSelection();
            if ( nSel >= 0 ) {
                if ( 1 == GetParent()->SendMessage(UD_LC_QUERY_ROW_DELETE,nSel,(LPARAM)this) ) {
                    DeleteItem(nSel);
                    GetParent()->SendMessage(UD_LC_ROW_DELETED,nSel,(LPARAM)this);
                }
            }
        }
        break;
    case VK_SUBTRACT:
        {
            int nSel = GetFirstSelection();
            if ( nSel > 0 && 1 == GetParent()->SendMessage(UD_LC_QUERY_ROW_MOVE,nSel,(LPARAM)this) ) {
                SwapRow(nSel-1,nSel);
                SetSelection(nSel-1,false);

                RowMovedInfo rmi;
                rmi.iNewIndex = nSel-1;
                rmi.iOldIndex = nSel;
                GetParent()->SendMessage( UD_LC_ROW_MOVED, (WPARAM)&rmi, (LPARAM)this );
            }
        }
        break;
    case VK_ADD:
        {
            int nSel = GetFirstSelection();
            if ( nSel >= 0 && nSel < GetItemCount() - 1
                && 1 == GetParent()->SendMessage(UD_LC_QUERY_ROW_MOVE,nSel,(LPARAM)this) )
            {
                SwapRow(nSel,nSel+1);
                SetSelection(nSel+1,false);

                RowMovedInfo rmi;
                rmi.iNewIndex = nSel+1;
                rmi.iOldIndex = nSel;
                GetParent()->SendMessage( UD_LC_ROW_MOVED, (WPARAM)&rmi, (LPARAM)this );
            }
        }
        break;
    default:
        CListCtrl::OnKeyDown(nChar,nRepCnt,nFlags) ;
        break;
    }
}

int CEditListCtrl::GetFirstSelection()
{
    int nCount = GetItemCount();
    for ( int i = 0; i < nCount; i++ ) {
        if ( GetItemState( i, LVIS_SELECTED ) == LVIS_SELECTED ) return i;
    }
    return -1;
}

int CEditListCtrl::SetSelection( int index, bool bMulti )
{
    int nCount = GetItemCount();
    if ( !bMulti ) {
        for ( int i = 0; i < nCount; i++ ) {
            SetItemState( i, 0, LVIS_SELECTED );
        }
    }
    if ( index < nCount ) {
        if ( SetItemState( index, LVIS_SELECTED, LVIS_SELECTED ) ) return index;
    }
    return -1;
}

int CEditListCtrl::InsertColumn(int nCol, LPTSTR sName, int nWidth, COLORREF crTextColor, COLORREF crBkColor, int nFormat, PFNLVCOMPARE pfnCompare )
{
    LVCOLUMN lvColData;
    lvColData.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_FMT ;
    lvColData.cx = nWidth;
    lvColData.pszText = sName;
    lvColData.fmt = nFormat;
    nCol = CListCtrl::InsertColumn( nCol, &lvColData );
    if ( nCol >= 0 ) {
        COLUMNDATA* hid = new COLUMNDATA;
        hid->ct = CT_NONE;
        hid->crTextColor = crTextColor;
        hid->crBkColor = crBkColor;
        hid->pfnCompare = pfnCompare;
        HDITEM hdi;
        hdi.mask = HDI_LPARAM;
        hdi.lParam = (LPARAM)hid;
        m_hcSortedHeader.SetItem(nCol,&hdi);
    }
    return nCol;
}

int CEditListCtrl::InsertEditBoxColumn( int nCol, LPTSTR sName, int nWidth, COLORREF crTextColor, COLORREF crBkColor, int nFormat, PFNLVCOMPARE pfnCompare )
{
    LVCOLUMN lvColData;
    lvColData.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_FMT;
    lvColData.cx = nWidth;
    lvColData.pszText = sName;
    lvColData.fmt = nFormat;
    nCol = CListCtrl::InsertColumn( nCol, &lvColData );
    if ( nCol >= 0 ) {
        COLUMNDATA* hid = new COLUMNDATA;
        hid->ct = CT_EDITBOX;
        hid->crTextColor = crTextColor;
        hid->crBkColor = crBkColor;
        hid->pfnCompare = pfnCompare;
        HDITEM hdi;
        hdi.mask = HDI_LPARAM;
        hdi.lParam = (LPARAM)hid;
        m_hcSortedHeader.SetItem(nCol,&hdi);
    }
    return nCol;
}

int CEditListCtrl::InsertComboBoxColumn(  int nCol, LPTSTR sName, int nWidth, COLORREF crTextColor, COLORREF crBkColor, LPCTSTR lpszItems, int nFormat, PFNLVCOMPARE pfnCompare )
{
    LVCOLUMN lvColData;
    lvColData.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_FMT ;
    lvColData.cx = nWidth;
    lvColData.pszText = sName;
    lvColData.fmt = nFormat;
    nCol = CListCtrl::InsertColumn( nCol, &lvColData );
    if ( nCol >= 0 ) {
        COLUMNDATA* hid = new COLUMNDATA;
        hid->ct = CT_COMBOBOX;
        hid->crTextColor = crTextColor;
        hid->crBkColor = crBkColor;

        CString strList(lpszItems);
        for ( int i = 0, nStart = 0, nEnd = 0; ; i++ ) {
            nEnd = strList.Find('|',nStart);
            if ( nEnd < 0 ) nEnd = strList.GetLength();
            if ( nStart >= nEnd ) break;
            hid->astrComboItem.Add( strList.Mid(nStart,nEnd-nStart) );
            nStart = nEnd + 1;
        }

        hid->pfnCompare = pfnCompare;

        HDITEM hdi;
        hdi.mask = HDI_LPARAM;
        hdi.lParam = (LPARAM)hid;
        m_hcSortedHeader.SetItem(nCol,&hdi);
    }
    return nCol;
}

int    CEditListCtrl::InsertHexEditBoxColumn( int nCol, LPTSTR sName, int nWidth, COLORREF crTextColor, COLORREF crBkColor, int nDigitCount, int nFormat, PFNLVCOMPARE pfnCompare )
{
    LVCOLUMN lvColData;
    lvColData.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_FMT ;
    lvColData.cx = nWidth;
    lvColData.pszText = sName;
    lvColData.fmt = nFormat;
    nCol = CListCtrl::InsertColumn( nCol, &lvColData );
    if ( nCol >= 0 ) {
        COLUMNDATA* hid = new COLUMNDATA;
        hid->ct = CT_HEXEDITBOX;
        hid->crTextColor = crTextColor;
        hid->crBkColor = crBkColor;
        hid->count = nDigitCount;
        hid->pfnCompare = pfnCompare;
        HDITEM hdi;
        hdi.mask = HDI_LPARAM;
        hdi.lParam = (LPARAM)hid;
        m_hcSortedHeader.SetItem(nCol,&hdi);
    }
    return nCol;
}

int    CEditListCtrl::InsertProgressBarColumn( int nCol, LPTSTR sName, int nWidth, COLORREF crColor1, COLORREF crColor2, PFNLVCOMPARE pfnCompare )
{
    LVCOLUMN lvColData;
    lvColData.mask = LVCF_TEXT | LVCF_WIDTH ;
    lvColData.cx = nWidth;
    lvColData.pszText = sName;
    nCol = CListCtrl::InsertColumn( nCol, &lvColData );
    if ( nCol >= 0 ) {
        COLUMNDATA* hid = new COLUMNDATA;
        hid->ct = CT_PROGRESSBAR;
        hid->crPBColor1 = crColor1;
        hid->crPBColor2 = crColor2;
        hid->pfnCompare = pfnCompare;
        HDITEM hdi;
        hdi.mask = HDI_LPARAM;
        hdi.lParam = (LPARAM)hid;
        m_hcSortedHeader.SetItem(nCol,&hdi);
    }
    return nCol;
}

int    CEditListCtrl::InsertCheckBoxColumn( int nCol, LPTSTR sName, int nWidth, COLORREF crBkColor, PFNLVCOMPARE pfnCompare )
{
    LVCOLUMN lvColData;
    lvColData.mask = LVCF_TEXT | LVCF_WIDTH ;
    lvColData.cx = nWidth;
    lvColData.pszText = sName;
    nCol = CListCtrl::InsertColumn( nCol, &lvColData );
    if ( nCol >= 0 ) {
        COLUMNDATA* hid = new COLUMNDATA;
        hid->ct = CT_CHECKBOX;
        hid->crBkColor = crBkColor;
        hid->pfnCompare = pfnCompare;
        HDITEM hdi;
        hdi.mask = HDI_LPARAM;
        hdi.lParam = (LPARAM)hid;
        m_hcSortedHeader.SetItem(nCol,&hdi);
    }
    return nCol;
}

int CEditListCtrl::InsertColumn( const COLUMNINIT columns[], int nCount )
{
    int i;
    for ( i = 0; i < nCount; i++ ) {
        TCHAR achBuf[MAX_PATH];
        ::LoadString( NULL, columns[i].nNameID, achBuf, MAX_PATH );

        switch ( columns[i].ct ) {
            case CT_NONE:
                InsertColumn( columns[i].nCol, achBuf,
                    columns[i].nWidth, columns[i].color1, columns[i].color2 ); 
                break;
            case CT_EDITBOX:
                InsertEditBoxColumn( columns[i].nCol, achBuf,
                    columns[i].nWidth, columns[i].color1, columns[i].color2 );
                break;
            case CT_COMBOBOX:
                InsertComboBoxColumn( columns[i].nCol, achBuf,
                    columns[i].nWidth, columns[i].color1, columns[i].color2, columns[i].cbdata );
                break;
            case CT_HEXEDITBOX:
                InsertHexEditBoxColumn( columns[i].nCol, achBuf,
                    columns[i].nWidth, columns[i].color1, columns[i].color2, columns[i].nItemSpec );
            case CT_PROGRESSBAR:
                InsertProgressBarColumn(  columns[i].nCol, achBuf,
                    columns[i].nWidth, columns[i].color1, columns[i].color2 );
            case CT_CHECKBOX: 
                InsertCheckBoxColumn( columns[i].nCol, achBuf,
                    columns[i].nWidth, columns[i].color1 );
            default:
                break;
        }
    }

    return i;
}

void CEditListCtrl::SetExtendedStyle( DWORD dwExStyle )
{
    DWORD dwStyle = CListCtrl::GetExtendedStyle();
    dwStyle |= dwExStyle;
    CListCtrl::SetExtendedStyle( dwStyle );
}

void CEditListCtrl::SetStyle( DWORD dwStyle )
{
    ModifyStyle( LVS_TYPEMASK, dwStyle & LVS_TYPEMASK );
}

void CEditListCtrl::GenerateNo( int first, int num )
{
    int nItemCount = CListCtrl::GetItemCount();
    if ( first >= nItemCount ) return;
    if ( num < 0 || first + num > nItemCount ) {
        num = nItemCount - first;
    }

    TCHAR buf[16];
    for ( int i = 0; i < num; i++ ) {
        _itot_s( i+first+1, buf, 10 );
        CListCtrl::SetItemText( i + first, 0, buf );
    }
}

void CEditListCtrl::SwapRow( int nRow1, int nRow2 )
{
    int nCount = GetItemCount();
    if ( nRow1 >= nCount || nRow2 >= nCount ) return;

    CHeaderCtrl* pHeader = (CHeaderCtrl*)GetDlgItem(0);
    int nColumnCount = pHeader->GetItemCount();

    CString str1, str2;
    for ( int i = 0; i < nColumnCount; i++ ) {
        str1 = GetItemText(nRow1,i);
        str2 = GetItemText(nRow2,i);
        SetItemText(nRow1,i,str2);
        SetItemText(nRow2,i,str1);
    }

    DWORD_PTR dwItemData1 = GetItemData(nRow1);
    DWORD_PTR dwItemData2 = GetItemData(nRow2);
    SetItemData(nRow1,dwItemData2);
    SetItemData(nRow2,dwItemData1);
    UINT state1 = GetItemState(nRow1,0xffffffff);
    UINT state2 = GetItemState(nRow2,0xffffffff);
    SetItemState(nRow1,state2,0xffffffff);
    SetItemState(nRow2,state1,0xffffffff);
}

void CEditListCtrl::SetImageList( UINT nBitmapID, int cx, int nGrow, COLORREF crMask, int nImageList )
{
    switch ( nImageList ) {
    case -1: // for Header Control
        {
            m_ilHeader.DeleteImageList();
            m_ilHeader.Create( nBitmapID, cx, nGrow, crMask );
            m_hcSortedHeader.SetImageList(&m_ilHeader);
            HDITEM hdi;
            for ( int i = 0; i < m_hcSortedHeader.GetItemCount(); i++ ) {
                hdi.fmt = HDI_FORMAT;
                m_hcSortedHeader.GetItem(i,&hdi);
                hdi.fmt |= HDF_IMAGE;
                hdi.iImage = i;
                hdi.mask |= HDI_IMAGE | HDI_FORMAT;
                m_hcSortedHeader.SetItem(i,&hdi);
            }
        }
        break;
    case LVSIL_NORMAL:
        m_ilNormal.DeleteImageList();
        m_ilNormal.Create( nBitmapID, cx, nGrow, crMask );
        CListCtrl::SetImageList( &m_ilNormal, nImageList );
        break;
    case LVSIL_SMALL:
        m_ilSmall.DeleteImageList();
        m_ilSmall.Create( nBitmapID, cx, nGrow, crMask );
        CListCtrl::SetImageList( &m_ilSmall, nImageList );
        break;
    case LVSIL_STATE:
        m_ilState.DeleteImageList();
        m_ilState.Create( nBitmapID, cx, nGrow, crMask );
        CListCtrl::SetImageList( &m_ilState, nImageList );
        break;
    }
}

int    CEditListCtrl::SetCurSelInComboBoxColumn( int nItem, int nCol, int nSelect )
{
    int nCount = GetItemCount();
    if ( nItem < 0 || nItem >= nCount )
        return CB_ERR;

    CHeaderCtrl* pHeaderCtrl = GetHeaderCtrl();
    if ( nCol < 0 || nCol >= pHeaderCtrl->GetItemCount() )
        return CB_ERR;

    HDITEM hdi;
    hdi.mask = HDI_LPARAM;
    pHeaderCtrl->GetItem( nCol, &hdi );
    COLUMNDATA* pcd = (COLUMNDATA*)hdi.lParam;

    if ( pcd->ct != CT_COMBOBOX )
        return CB_ERR;

    if ( nSelect < 0 || nSelect >= pcd->astrComboItem.GetSize() )
        return CB_ERR;

    SetItemText( nItem, nCol, pcd->astrComboItem[nSelect] );

    return nSelect;
}

int    CEditListCtrl::GetCurSelInComboBoxColumn( int nItem, int nCol )
{
    int nCount = GetItemCount();
    if ( nItem < 0 || nItem >= nCount )
        return CB_ERR;

    CHeaderCtrl* pHeaderCtrl = GetHeaderCtrl();
    if ( nCol < 0 || nCol >= pHeaderCtrl->GetItemCount() )
        return CB_ERR;

    HDITEM hdi;
    hdi.mask = HDI_LPARAM;
    pHeaderCtrl->GetItem( nCol, &hdi );
    COLUMNDATA* pcd = (COLUMNDATA*)hdi.lParam;

    if ( pcd->ct != CT_COMBOBOX )
        return CB_ERR;

    CString str = GetItemText( nItem, nCol );

    for ( int i = 0; i < pcd->astrComboItem.GetSize(); i++ ) {
        if ( pcd->astrComboItem[i] == str )
            return i;
    }

    return CB_ERR;
}

int    CEditListCtrl::GetIndexInComboBoxColumn( int nCol, LPCTSTR lpszText )
{
    CHeaderCtrl* pHeaderCtrl = GetHeaderCtrl();
    if ( nCol < 0 || nCol >= pHeaderCtrl->GetItemCount() )
        return CB_ERR;

    HDITEM hdi;
    hdi.mask = HDI_LPARAM;
    pHeaderCtrl->GetItem( nCol, &hdi );
    COLUMNDATA* pcd = (COLUMNDATA*)hdi.lParam;

    if ( pcd->ct != CT_COMBOBOX )
        return CB_ERR;
    
    for ( int i = 0; i < pcd->astrComboItem.GetSize(); i++ ) {
        if ( pcd->astrComboItem[i] == lpszText )
            return i;
    }

    return CB_ERR;
}

int    CEditListCtrl::SetCheckInCheckBoxColumn( int nItem, int nCol, int nCheck )
{
    CHeaderCtrl* pHeaderCtrl = GetHeaderCtrl();
    if ( nCol < 0 || nCol >= pHeaderCtrl->GetItemCount() )
        return -1;

    HDITEM hdi;
    hdi.mask = HDI_LPARAM;
    pHeaderCtrl->GetItem( nCol, &hdi );
    COLUMNDATA* pcd = (COLUMNDATA*)hdi.lParam;

    if ( pcd->ct != CT_CHECKBOX )
        return -1;
    
    switch ( nCheck ) {
    case 0:
        SetItemText(nItem,nCol,_T(" "));
        return 0;
    case 1:
        SetItemText(nItem,nCol,_T("v"));
        return 1;
    }
    return -1;
}

int    CEditListCtrl::GetCheckInCheckBoxColumn( int nItem, int nCol )
{
    CHeaderCtrl* pHeaderCtrl = GetHeaderCtrl();
    if ( nCol < 0 || nCol >= pHeaderCtrl->GetItemCount() )
        return -1;

    HDITEM hdi;
    hdi.mask = HDI_LPARAM;
    pHeaderCtrl->GetItem( nCol, &hdi );
    COLUMNDATA* pcd = (COLUMNDATA*)hdi.lParam;

    if ( pcd->ct != CT_CHECKBOX )
        return -1;

    CString strItem = GetItemText(nItem,nCol);
    if ( strItem == " " )
        return 0;
    else if ( strItem == "v" )
        return 1;

    return -1;
}

int CEditListCtrl::GetItemValue( int nItem, int nCol, int nBase )
{
    CString strItem = GetItemText( nItem, nCol );
    TCHAR* endptr;
    return _tcstol( strItem, &endptr, nBase );
}

BOOL CEditListCtrl::SetItemTextEx( int nItem, int nCol, LPCTSTR format, ... )
{
    va_list args;
    va_start(args, format);

    int nBuf;
    TCHAR szBuffer[512];

    nBuf = _vstprintf_s(szBuffer, format, args);
    ASSERT(nBuf < (sizeof(szBuffer)/sizeof(szBuffer[0])));
    va_end(args);

    return SetItemText(nItem,nCol,szBuffer);
}

void CEditListCtrl::Sort( int iColumn, bool bAscending )
{
    HDITEM hdi;
    hdi.mask = HDI_LPARAM;
    m_hcSortedHeader.GetItem( iColumn, &hdi );
    COLUMNDATA* pcd = (COLUMNDATA*)hdi.lParam;
    if ( NULL == pcd->pfnCompare )
        return;

    m_iSortColumn = iColumn;
    m_bSortAscending = bAscending;

    // show the appropriate arrow in the header control.
    m_hcSortedHeader.SetSortArrow( m_iSortColumn, m_bSortAscending );

    VERIFY( SortItems( pcd->pfnCompare, reinterpret_cast<DWORD_PTR>( this ) ) );
}

void CEditListCtrl::AutoSizeColumns( int col )
{
    SetRedraw(false);
    int mincol = col < 0 ? 0 : col;
    int maxcol = col < 0 ? m_hcSortedHeader.GetItemCount() - 1 : col;
    for ( col = mincol; col <= maxcol; col++ ) {
        SetColumnWidth(col,LVSCW_AUTOSIZE_USEHEADER);
    }
    SetRedraw();
}

CImageList* CEditListCtrl::CreateDragImageEx(LPPOINT lpPoint)
{
    if (GetSelectedCount() <= 0)
        return NULL; // no row selected

    CRect rectSingle;
    CRect rectComplete(0,0,0,0);

    // Determine List Control Client width size
    GetClientRect(rectSingle);
    int nWidth  = rectSingle.Width();

    // Start and Stop index in view area
    int nIndex = GetTopIndex() - 1;
    int nBottomIndex = GetTopIndex() + GetCountPerPage() - 1;
    if (nBottomIndex > (GetItemCount() - 1))
        nBottomIndex = GetItemCount() - 1;

    // Determine the size of the drag image (limite for 
    // rows visibled and Client width)
    while ((nIndex = GetNextItem(nIndex, LVNI_SELECTED)) != -1) {
        if (nIndex > nBottomIndex)
            break; 

        GetItemRect(nIndex, rectSingle, LVIR_BOUNDS);

        if (rectSingle.left < 0) 
            rectSingle.left = 0;

        if (rectSingle.right > nWidth)
            rectSingle.right = nWidth;

        rectComplete.UnionRect(rectComplete, rectSingle);
    }

    CClientDC dcClient(this);
    CDC dcMem;
    CBitmap Bitmap;

    if (!dcMem.CreateCompatibleDC(&dcClient))
        return NULL;

    if (!Bitmap.CreateCompatibleBitmap(&dcClient, 
        rectComplete.Width(), 
        rectComplete.Height()))
        return NULL;

    CBitmap *pOldMemDCBitmap = dcMem.SelectObject(&Bitmap);
    // Use green as mask color
    dcMem.FillSolidRect(0, 0, 
        rectComplete.Width(), 
        rectComplete.Height(), 
        RGB(0,255,0));

    // Paint each DragImage in the DC
    nIndex = GetTopIndex() - 1;
    while ((nIndex = GetNextItem(nIndex, LVNI_SELECTED)) != -1) {   
        if (nIndex > nBottomIndex)
            break;

        CPoint pt;
        CImageList* pSingleImageList = CreateDragImage(nIndex, &pt);

        if (pSingleImageList) {
            GetItemRect(nIndex, rectSingle, LVIR_BOUNDS);

            pSingleImageList->Draw(&dcMem, 0, 
                CPoint(rectSingle.left - rectComplete.left,
                rectSingle.top - rectComplete.top), 
                ILD_MASK);

            pSingleImageList->DeleteImageList();
            delete pSingleImageList;
        }
    }

    dcMem.SelectObject(pOldMemDCBitmap);
    CImageList* pCompleteImageList = new CImageList;
    pCompleteImageList->Create(rectComplete.Width(), 
        rectComplete.Height(), 
        ILC_COLOR | ILC_MASK, 
        0, 1);
    
    // Green is used as mask color
    pCompleteImageList->Add(&Bitmap, RGB(0, 255, 0)); 
    Bitmap.DeleteObject();

    if (lpPoint) {
        lpPoint->x = rectComplete.left;
        lpPoint->y = rectComplete.top;
    }

    return pCompleteImageList;
}

void CEditListCtrl::SaveColumnInfo( LPCTSTR lpszSection )
{
    int nColumnCount = GetHeaderCtrl()->GetItemCount();

    CString str;
    for ( int i = 0; i < nColumnCount; i++ ) {
        str.Format( _T("Col%02dWidth"), i );
        AfxGetApp()->WriteProfileInt( lpszSection, str, GetColumnWidth(i) );
    }
}

void CEditListCtrl::LoadColumnInfo( LPCTSTR lpszSection )
{
    int nColumnCount = GetHeaderCtrl()->GetItemCount();

    for ( int i = 0; i < nColumnCount; i++ ) {
        CString str;
        str.Format( _T("Col%02dWidth"), i );
        int nWidth = GetColumnWidth(i);
        nWidth = AfxGetApp()->GetProfileInt( lpszSection, str, nWidth );
        SetColumnWidth( i, nWidth );
    }
}
