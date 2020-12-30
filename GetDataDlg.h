#pragma once
#include "afxcmn.h"
#include "afxwin.h"

#include "HidDevice.h"
#include "EditListCtrl.h"
#include "HexEdit.h"

// CGetDataDlg dialog

class CGetDataDlg : public CDialogEx
{
    DECLARE_DYNAMIC(CGetDataDlg)

    CString m_strTitle;
    PWRITE_DATA_STRUCT m_pWriteData;
    int m_nCount;

public:
    CGetDataDlg(LPCTSTR pszTitle, PWRITE_DATA_STRUCT pWriteData, int count, CWnd* pParent = NULL);   // standard constructor
    virtual ~CGetDataDlg();

// Dialog Data
    enum { IDD = IDD_GETDATA };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    DECLARE_MESSAGE_MAP()
    afx_msg void OnBnClickedBtnSend();

    virtual BOOL OnInitDialog();

public:

    CEditListCtrl m_lcFeatures;
    CHexEdit m_ecData;
};
