// WriteFeatureDlg.cpp : implementation file
//

#include "stdafx.h"
#include "HidCollections.h"
#include "GetDataDlg.h"
#include "afxdialogex.h"


// CGetDataDlg dialog

IMPLEMENT_DYNAMIC(CGetDataDlg, CDialogEx)

CGetDataDlg::CGetDataDlg(LPCTSTR pszTitle, PWRITE_DATA_STRUCT pWriteData, int count, CWnd* pParent /*=NULL*/)
    : CDialogEx(CGetDataDlg::IDD, pParent)
{
    m_strTitle = pszTitle;
    m_pWriteData = pWriteData;
    m_nCount = count;
}

CGetDataDlg::~CGetDataDlg()
{
}

void CGetDataDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LC_FEATURES, m_lcFeatures);
    DDX_Control(pDX, IDC_EC_DATA, m_ecData);
}


BEGIN_MESSAGE_MAP(CGetDataDlg, CDialogEx)
    ON_BN_CLICKED(IDC_BTN_SEND, &CGetDataDlg::OnBnClickedBtnSend)
END_MESSAGE_MAP()


// CGetDataDlg message handlers


BOOL CGetDataDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    // TODO:  Add extra initialization here
    SetWindowText(m_strTitle);
    m_lcFeatures.InsertColumn( 0, _T("Feature Buttons && Values"), 0, RGB(0,0,0), RGB(0xe0,0xe0,0xe0) );
    m_lcFeatures.InsertEditBoxColumn( 1, _T("Output"), 0, RGB(0,0,0), RGB(0xe0,0xe0,0xe0) );

    m_lcFeatures.SetExtendedStyle(LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES);

    for (int i = 0; i < m_nCount; i++) {
        m_lcFeatures.InsertItem( i, m_pWriteData[i].szLabel );
    }

    m_lcFeatures.SetColumnWidth( 0, LVSCW_AUTOSIZE_USEHEADER);
    m_lcFeatures.SetColumnWidth( 1, LVSCW_AUTOSIZE_USEHEADER);

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

void CGetDataDlg::OnBnClickedBtnSend()
{
    PWRITE_DATA_STRUCT p = m_pWriteData;
    for (int i = 0; i < m_nCount; i++) {
        m_lcFeatures.GetItemText( i, 1, p->szValue, MAX_PATH );
        p++;
    }

    OnOK();
}



