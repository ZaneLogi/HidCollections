#if !defined(AFX_EDITLISTCTRL_H__77038058_41F8_11D4_8886_0050BAE3CB28__INCLUDED_)
#define AFX_EDITLISTCTRL_H__77038058_41F8_11D4_8886_0050BAE3CB28__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EditListCtrl.h : header file
//

#define UD_LC_QUERY_ITEM_CHANGE    (WM_USER+100) // not yet implemented
#define UD_LC_ITEM_CHANGED        (WM_USER+101)
#define UD_LC_QUERY_ROW_MOVE    (WM_USER+102)
#define UD_LC_ROW_MOVED            (WM_USER+103)
#define UD_LC_QUERY_ROW_INSERT    (WM_USER+104)
#define UD_LC_ROW_INSERTED        (WM_USER+105)
#define UD_LC_QUERY_ROW_DELETE    (WM_USER+106)
#define UD_LC_ROW_DELETED        (WM_USER+107)
#define UD_LC_ITEM_POSTPAINT    (WM_USER+108)
#define UD_LC_SUBITEM_DRAW        (WM_USER+109)

class CEditListCtrl;

struct ItemChangedInfo
{
    int iItem;
    int iSubItem;
    union {
        int iComboSel;
        int iCheckState;
    };
    CEditListCtrl* pList;
};

struct RowMovedInfo
{
    int iOldIndex;
    int iNewIndex;
};

enum COLUMNTYPE { CT_NONE = 0, CT_EDITBOX, CT_COMBOBOX, CT_HEXEDITBOX, CT_PROGRESSBAR, CT_CHECKBOX };

struct COLUMNDATA
{
    COLUMNTYPE        ct;
    union {
        COLORREF        crTextColor;
        COLORREF        crPBColor1;
    };
    union {
        COLORREF        crBkColor;
        COLORREF        crPBColor2;
    };
    union {
        int            count; // number count for HexEditBox
        int            state; // states for CheckBox
    };
    CStringArray    astrComboItem;
    PFNLVCOMPARE    pfnCompare;
};

struct COLUMNINIT {
    int                nCol;
    int                nNameID;
    int                nWidth;
    COLUMNTYPE        ct;
    COLORREF        color1;
    COLORREF        color2;
    int                nItemSpec;
    LPCTSTR            cbdata;
};

/////////////////////////////////////////////////////////////////////////////
// CInPlaceEditBox window

class CInPlaceEditBox : public CEdit
{
// Construction
public:
    CInPlaceEditBox(int iItem, int iSubItem, CString sInitText);

// Attributes
public:

// Operations
public:

// Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CInPlaceEditBox)
    public:
    virtual BOOL PreTranslateMessage(MSG* pMsg);
    //}}AFX_VIRTUAL

// Implementation
public:
    virtual ~CInPlaceEditBox();

    // Generated message map functions
protected:
    void ResizeToFit();

    //{{AFX_MSG(CInPlaceEditBox)
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnKillFocus(CWnd* pNewWnd);
    afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg void OnNcDestroy();
    afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
    //}}AFX_MSG

    DECLARE_MESSAGE_MAP()
protected:
    int m_iItem;
    int m_iSubItem;
    CString m_sInitText;
    BOOL    m_bESC;         // To indicate whether ESC key was pressed
};

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// CInPlaceHexEditBox window

class CInPlaceHexEditBox : public CInPlaceEditBox
{
// Construction
public:
    CInPlaceHexEditBox( int count, int iItem, int iSubItem, CString sInitText );

// Attributes
public:
    int m_nDigitCount;

// Operations
public:

// Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CInPlaceHexEditBox)
    //}}AFX_VIRTUAL

// Implementation
public:
    virtual ~CInPlaceHexEditBox();

    // Generated message map functions
protected:
    //{{AFX_MSG(CInPlaceHexEditBox)
    afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg int  OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
    //}}AFX_MSG

    DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// CInPlaceComboBox window

class CInPlaceComboBox : public CComboBox
{
// Construction
public:
    CInPlaceComboBox(int iItem, int iSubItem, CString sInitText, const CStringArray& astrComboItem );

// Attributes
public:

// Operations
public:

// Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CInPlaceList)
    public:
    virtual BOOL PreTranslateMessage(MSG* pMsg);
    //}}AFX_VIRTUAL

// Implementation
public:
    virtual ~CInPlaceComboBox();

    // Generated message map functions
protected:
    //{{AFX_MSG(CInPlaceComboBox)
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnKillFocus(CWnd* pNewWnd);
    afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg void OnNcDestroy();
    afx_msg void OnCloseup();
    //}}AFX_MSG

    DECLARE_MESSAGE_MAP()
private:
    int     m_iItem;
    int     m_iSubItem;
    CStringList m_lstItems;
    int     m_nSel;
    BOOL    m_bESC;                // To indicate whether ESC key was pressed
};

/////////////////////////////////////////////////////////////////////////////

#include "SortedHeaderCtrl.h"

/////////////////////////////////////////////////////////////////////////////
// CEditListCtrl window

class CEditListCtrl : public CListCtrl
{
// Construction
public:
    CEditListCtrl();

// Attributes
public:
protected:
    CImageList    m_ilHeader;
    CImageList    m_ilNormal;
    CImageList    m_ilSmall;
    CImageList    m_ilState;

    CSortedHeaderCtrl m_hcSortedHeader;

    int            m_iSortColumn;
    bool        m_bSortAscending;

    int            m_nDragIndex;
    int            m_nDropIndex;
    bool        m_bDragging;
    CImageList* m_pDragImage;

// Operations
public:
    static int CALLBACK CompareFunction( LPARAM lParam1, LPARAM lParam2, LPARAM lParamData );

    void    EnableEditLabels(bool bEnabled=true);

    // Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CEditListCtrl)
    protected:
    virtual void PreSubclassWindow();
    //}}AFX_VIRTUAL

// Implementation
public:
    virtual        ~CEditListCtrl();

    void        LoadColumnInfo(LPCTSTR lpszSection);
    void        SaveColumnInfo(LPCTSTR lpszSection);

    void        EditSubLabel( int nItem, int nCol );
    int            HitTestEx(CPoint &point, int *col) const;
    int            GetFirstSelection();
    int            SetSelection( int index, bool bMulti );
    
    int            InsertColumn( int nCol, LPTSTR sName, int nWidth, COLORREF crTextColor, COLORREF crBkColor, int nFormat = LVCFMT_LEFT, PFNLVCOMPARE pfnCompare = NULL );
    int            InsertEditBoxColumn( int nCol, LPTSTR sName, int nWidth, COLORREF crTextColor, COLORREF crBkColor, int nFormat = LVCFMT_LEFT, PFNLVCOMPARE pfnCompare = NULL );
    int            InsertComboBoxColumn(  int nCol, LPTSTR sName, int nWidth, COLORREF crTextColor, COLORREF crBkColor, LPCTSTR lpszItems, int nFormat = LVCFMT_LEFT, PFNLVCOMPARE pfnCompare = NULL );
    int            InsertHexEditBoxColumn( int nCol, LPTSTR sName, int nWidth, COLORREF crTextColor, COLORREF crBkColor, int nDigitCount, int nFormat = LVCFMT_LEFT, PFNLVCOMPARE pfnCompare = NULL );
    int            InsertProgressBarColumn( int nCol, LPTSTR sName, int nWidth, COLORREF crColor1, COLORREF crColor2, PFNLVCOMPARE pfnCompare = NULL );
    int            InsertCheckBoxColumn( int nCol, LPTSTR sName, int nWidth, COLORREF crBkColor, PFNLVCOMPARE pfnCompare = NULL );
    int            InsertColumn( const COLUMNINIT columns[], int nCount );

    void        SetExtendedStyle( DWORD dwExStyle = LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP );
    void        SetStyle( DWORD dwStyle );
    void        GenerateNo( int fisrt, int num = -1 );
    void        SwapRow( int nRow1, int nRow2 );
    void        SetImageList( UINT nBitmapID, int cx, int nGrow, COLORREF crMask, int nImageList );
    int            SetCurSelInComboBoxColumn( int nItem, int nCol, int nSelect );
    int            GetCurSelInComboBoxColumn( int nItem, int nCol );
    int            GetIndexInComboBoxColumn( int nCol, LPCTSTR lpszText );
    int            SetCheckInCheckBoxColumn( int nItem, int nCol, int nCheck );
    int            GetCheckInCheckBoxColumn( int nItem, int nCol );
    int            GetItemValue( int nItem, int nCol, int nBase = 10 );
    BOOL        SetItemTextEx( int nItem, int nCol, LPCTSTR format, ... );
    void        Sort( int iColumn, bool bAscending );
    void        AutoSizeColumns( int col = -1 );
    CImageList*    CreateDragImageEx(LPPOINT lpPoint);

protected:
    bool        DropItemOnList();

    // Generated message map functions
protected:
    //{{AFX_MSG(CEditListCtrl)
    afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
    afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
    afx_msg void OnBeginLabelEdit(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnEndLabelEdit(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnDestroy();
    afx_msg void OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnColumnClick(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnBeginDrag(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
    //}}AFX_MSG

    DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EDITLISTCTRL_H__77038058_41F8_11D4_8886_0050BAE3CB28__INCLUDED_)
