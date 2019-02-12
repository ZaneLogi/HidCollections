#if !defined(AFX_SORTEDHEADERCTRL_H__A2CA6CBD_F3F9_402E_8816_F08BA1E1AF8C__INCLUDED_)
#define AFX_SORTEDHEADERCTRL_H__A2CA6CBD_F3F9_402E_8816_F08BA1E1AF8C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SortedHeaderCtrl.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSortedHeaderCtrl window

class CSortedHeaderCtrl : public CHeaderCtrl
{
// Construction
public:
    CSortedHeaderCtrl();

// Attributes
public:

// Operations
public:

// Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CSortedHeaderCtrl)
    //}}AFX_VIRTUAL

// Implementation
public:
    virtual ~CSortedHeaderCtrl();

    void SetSortArrow( const int iColumn, const bool bAscending );

    // Generated message map functions
protected:
    void DrawItem( LPDRAWITEMSTRUCT lpDrawItemStruct );

    int        m_iSortColumn;
    bool    m_bSortAscending;

    //{{AFX_MSG(CSortedHeaderCtrl)
        // NOTE - the ClassWizard will add and remove member functions here.
    //}}AFX_MSG

    DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SORTEDHEADERCTRL_H__A2CA6CBD_F3F9_402E_8816_F08BA1E1AF8C__INCLUDED_)
