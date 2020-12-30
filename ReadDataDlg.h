#pragma once

#include "HidDevice.h"
#include "afxwin.h"

#define WM_READ_DONE (WM_USER+100)
#define WM_DISPLAY_READ_DATA (WM_USER+101)

// CReadDataDlg dialog

class CReadDataDlg : public CDialogEx
{
    DECLARE_DYNAMIC(CReadDataDlg)

    CHidDevice* m_pHidDevice;

    HANDLE               m_readThread;
    HANDLE               m_displayEvent;
    CHidDevice           m_syncDevice;
    CHidDevice           m_asyncDevice;
    bool                 m_doAsyncReads;
    bool                 m_doSyncReads;
    bool                 m_bTerminateThread;
    bool                 m_bDoOneRead;
    bool                 m_bDoSyncRead;

    static DWORD WINAPI SynchReadThreadProc(PVOID pContext);
    static DWORD WINAPI AsynchReadThreadProc(PVOID pContext);

    void ReadAsync();

public:
    CReadDataDlg(CHidDevice* pHidDevice, CWnd* pParent = NULL);   // standard constructor
    virtual ~CReadDataDlg();

// Dialog Data
    enum { IDD = IDD_READDATA };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    virtual BOOL OnInitDialog();

    DECLARE_MESSAGE_MAP()
    afx_msg void OnBnClickedBtnReadAsynchCont();
    afx_msg void OnBnClickedBtnReadAsynchOnce();
    afx_msg void OnBnClickedBtnReadSynch();
    afx_msg void OnBnClickedOk();
    afx_msg void OnBnClickedCancel();
    afx_msg LRESULT OnDisplayData(WPARAM wParam = 0, LPARAM lParam = 0);
    afx_msg LRESULT OnReadDone(WPARAM wParam = 0, LPARAM lParam = 0);

public:
    CListBox m_lbOutput;
};
