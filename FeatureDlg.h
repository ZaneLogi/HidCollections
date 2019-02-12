#pragma once

#include "HidDevice.h"
#include "afxwin.h"

// CFeatureDlg dialog

class CFeatureDlg : public CDialogEx
{
    DECLARE_DYNAMIC(CFeatureDlg)

    CHidDevice* m_pHidDevice;
    PHIDP_VALUE_CAPS m_pValueCaps;

public:
    CFeatureDlg(CHidDevice* pHidDevice, PHIDP_VALUE_CAPS valueCaps, CWnd* pParent = NULL);   // standard constructor
    virtual ~CFeatureDlg();

// Dialog Data
    enum { IDD = IDD_FEATURES };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnBnClickedBtnRead();
    afx_msg void OnBnClickedBtnWrite();
    CListBox m_lbOutput;
};
