// ReadDataDlg.cpp : implementation file
//

#include "stdafx.h"
#include "HidCollections.h"
#include "ReadDataDlg.h"
#include "afxdialogex.h"

#define MAX_LB_ITEMS 250
#define READ_THREAD_TIMEOUT 1000

// CReadDataDlg dialog

IMPLEMENT_DYNAMIC(CReadDataDlg, CDialogEx)

CReadDataDlg::CReadDataDlg(CHidDevice* pHidDevice, CWnd* pParent /*=NULL*/)
    : CDialogEx(CReadDataDlg::IDD, pParent)
{
    m_pHidDevice = pHidDevice;
    m_readThread = NULL;
    m_bTerminateThread = false;
    m_bDoSyncRead = false;
}

CReadDataDlg::~CReadDataDlg()
{
}

void CReadDataDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LB_OUTPUT, m_lbOutput);
}


BEGIN_MESSAGE_MAP(CReadDataDlg, CDialogEx)
    ON_BN_CLICKED(IDC_BTN_READ_ASYNCH_CONT, &CReadDataDlg::OnBnClickedBtnReadAsynchCont)
    ON_BN_CLICKED(IDC_BTN_READ_ASYNCH_ONCE, &CReadDataDlg::OnBnClickedBtnReadAsynchOnce)
    ON_BN_CLICKED(IDC_BTN_READ_SYNCH, &CReadDataDlg::OnBnClickedBtnReadSynch)
    ON_MESSAGE(WM_READ_DONE, &CReadDataDlg::OnReadDone)
    ON_MESSAGE(WM_DISPLAY_READ_DATA, &CReadDataDlg::OnDisplayData)
    ON_BN_CLICKED(IDOK, &CReadDataDlg::OnBnClickedOk)
    ON_BN_CLICKED(IDCANCEL, &CReadDataDlg::OnBnClickedCancel)
END_MESSAGE_MAP()


// CReadDataDlg message handlers


void CReadDataDlg::OnBnClickedBtnReadAsynchCont()
{
    m_bDoOneRead = false;
    ReadAsync();
}


void CReadDataDlg::OnBnClickedBtnReadAsynchOnce()
{
    m_bDoOneRead = true;
    ReadAsync();
}

void CReadDataDlg::ReadAsync()
{
    DWORD threadID;

    if (NULL == m_readThread) {
        //
        // Start a new read thread
        //
        m_bTerminateThread = false;
        m_readThread = CreateThread(
            NULL,
            0,
            AsynchReadThreadProc,
            this,
            0,
            &threadID);

        if (NULL == m_readThread) {
            MessageBox(_T("Unable to create read thread"),
                _T("Application Error"), MB_ICONEXCLAMATION);
        } else {
            GetDlgItem(IDOK)->EnableWindow(FALSE);
            GetDlgItem(IDC_BTN_READ_SYNCH)->EnableWindow(FALSE);
            GetDlgItem(IDC_BTN_READ_ASYNCH_ONCE)->EnableWindow(m_bDoOneRead);
            GetDlgItem(IDC_BTN_READ_ASYNCH_CONT)->EnableWindow(!m_bDoOneRead);

            GetDlgItem(m_bDoOneRead?IDC_BTN_READ_ASYNCH_ONCE:IDC_BTN_READ_ASYNCH_CONT)->
                SetWindowText(_T("Stop Asynchronous Read"));
        }
    } else {
        //
        // Signal the terminate thread variable and
        //  wait for the read thread to complete.
        //
        m_bTerminateThread = true;
        WaitForSingleObject(m_readThread, INFINITE);
    }
}

void CReadDataDlg::OnBnClickedBtnReadSynch()
{
    DWORD threadID;
    m_readThread = CreateThread(
        NULL,
        0,
        SynchReadThreadProc,
        this,
        0,
        &threadID);

    if (NULL == m_readThread) {
        MessageBox(_T("Unable to create read thread"),
            _T("Application Error"), MB_ICONEXCLAMATION);
    } else {
        GetDlgItem(IDOK)->EnableWindow(FALSE);
        GetDlgItem(IDC_BTN_READ_SYNCH)->EnableWindow(FALSE);
        GetDlgItem(IDC_BTN_READ_ASYNCH_ONCE)->EnableWindow(FALSE);
        GetDlgItem(IDC_BTN_READ_ASYNCH_CONT)->EnableWindow(FALSE);
        m_bDoSyncRead = true;
    }
}

LRESULT CReadDataDlg::OnReadDone(WPARAM wParam, LPARAM lParam)
{
    m_bDoSyncRead = false;

    GetDlgItem(IDOK)->EnableWindow();
    GetDlgItem(IDC_BTN_READ_SYNCH)->EnableWindow(m_doSyncReads);
    GetDlgItem(IDC_BTN_READ_ASYNCH_ONCE)->EnableWindow(m_doAsyncReads);
    GetDlgItem(IDC_BTN_READ_ASYNCH_CONT)->EnableWindow(m_doAsyncReads);

    GetDlgItem(IDC_BTN_READ_ASYNCH_ONCE)->SetWindowText(
        _T("One Asynchronous Read"));

    GetDlgItem(IDC_BTN_READ_ASYNCH_CONT)->SetWindowText(
        _T("Continuous Asynchronous Read"));

    m_readThread = NULL;

    return 0;
}

LRESULT CReadDataDlg::OnDisplayData(WPARAM wParam, LPARAM lParam)
{
    CHidDevice* pHidDevice = &m_syncDevice;

    //
    // Display all the data stored in the Input data field for the device
    //
    PHID_DATA pData = pHidDevice->InputData;

    m_lbOutput.AddString(_T("-------------------------------------------"));

    if (m_lbOutput.GetCount() > MAX_LB_ITEMS) {
        m_lbOutput.DeleteString(0);
    }

    for (ULONG uLoop = 0; uLoop < pHidDevice->InputDataLength; uLoop++) {
        CString s;
        TCHAR* pStr = s.GetBufferSetLength(MAX_PATH);
        ReportToString(
            pData,
            pStr,
            MAX_PATH);
        s.ReleaseBuffer();

        int n = m_lbOutput.AddString(s);
        m_lbOutput.SetCurSel(n);
        if (m_lbOutput.GetCount() > MAX_LB_ITEMS) {
            m_lbOutput.DeleteString(0);
        }

        pData++;
    }

    SetEvent(m_displayEvent);
    return 0;
}

BOOL CReadDataDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    // TODO:  Add extra initialization here

    m_displayEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

    //
    // To do sync and async reads requires file handles with different
    //  attributes (ie. an async must be opened with the OVERLAPPED flag
    //  set).  The device node that was passed in the context parameter
    //  was not opened for reading.  Therefore, two more devices will
    //  be opened, one for async reads and one for sync reads.
    //
    m_doSyncReads = m_syncDevice.Open(m_pHidDevice->DevicePath,
        TRUE,
        FALSE,
        FALSE,
        FALSE);

    if (!m_doSyncReads) {
        MessageBox(_T("Unable to open device for synchronous reading"),
            _T("Application Error"), MB_ICONEXCLAMATION);
    }

    //
    // For asynchronous read, default to using the same information
    //    passed in as the lParam.  This is because data related to
    //    Ppd and such cannot be retrieved using the standard HidD_
    //    functions.  However, it is necessary to parse future reports.
    //
    m_doAsyncReads = m_asyncDevice.Open(m_pHidDevice->DevicePath,
        TRUE,
        FALSE,
        TRUE,
        FALSE);

    if (!m_doAsyncReads) {
        MessageBox(_T("Unable to open device for asynchronous reading"),
            _T("Application Error"), MB_ICONEXCLAMATION);
    }

    OnReadDone();

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

void CReadDataDlg::OnBnClickedOk()
{
    CloseHandle(m_displayEvent);

    if (m_doAsyncReads)
        m_asyncDevice.Close();

    if (m_doSyncReads)
        m_syncDevice.Close();

    CDialogEx::OnOK();
}

void CReadDataDlg::OnBnClickedCancel()
{
    if (m_bDoSyncRead) {
        return; // can't close the dialog because still in the sync-read process
    }

    m_bTerminateThread = true;
    WaitForSingleObject(m_readThread, INFINITE);

    OnBnClickedOk();
}

DWORD WINAPI
CReadDataDlg::SynchReadThreadProc(PVOID pContext)
{
    CReadDataDlg* pDlg = (CReadDataDlg*)pContext;
    CHidDevice* pHidDevice = &pDlg->m_syncDevice;

    pHidDevice->Read();

    UnpackReport(
        pHidDevice->InputReportBuffer,
        pHidDevice->Caps.InputReportByteLength,
        HidP_Input,
        pHidDevice->InputData,
        pHidDevice->InputDataLength,
        pHidDevice->Ppd);

    pDlg->PostMessage(WM_DISPLAY_READ_DATA);

    WaitForSingleObject(pDlg->m_displayEvent, INFINITE );

    pDlg->PostMessage(WM_READ_DONE);

    return 0;
}

DWORD WINAPI
CReadDataDlg::AsynchReadThreadProc(PVOID pContext)
{
    CReadDataDlg* pDlg = (CReadDataDlg*)pContext;
    CHidDevice* pHidDevice = &pDlg->m_asyncDevice;

    HANDLE  completionEvent;
    BOOL    readStatus;
    DWORD   waitStatus;

    //
    // Create the completion event to send to the the OverlappedRead routine
    //
    completionEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

    //
    // If NULL returned, then we cannot proceed any farther so we just exit the
    //  the thread
    //
    if (NULL == completionEvent) {
        goto AsyncRead_End;
    }

    //
    // Now we enter the main read loop, which does the following:
    //  1) Calls ReadOverlapped()
    //  2) Waits for read completion with a timeout just to check if
    //      the main thread wants us to terminate our the read request
    //  3) If the read fails, we simply break out of the loop
    //      and exit the thread
    //  4) If the read succeeds, we call UnpackReport to get the relevant
    //      info and then post a message to main thread to indicate that
    //      there is new data to display.
    //  5) We then block on the display event until the main thread says
    //      it has properly displayed the new data
    //  6) Look to repeat this loop if we are doing more than one read
    //      and the main thread has yet to want us to terminate
    //
    do
    {
        //
        // Call ReadOverlapped() and if the return status is TRUE, the ReadFile
        //  succeeded so we need to block on completionEvent, otherwise, we just
        //  exit
        //
        readStatus = pHidDevice->ReadOverlapped(completionEvent);
        if (!readStatus) {
            break;
        }

        while (!pDlg->m_bTerminateThread) {
            //
            // Wait for the completion event to be signaled or a timeout
            //
            waitStatus = WaitForSingleObject (completionEvent, READ_THREAD_TIMEOUT );

            //
            // If completionEvent was signaled, then a read just completed
            //   so let's leave this loop and process the data
            //
            if ( WAIT_OBJECT_0 == waitStatus) {
                break;
            }
        }

        //
        // Check the TerminateThread again...If it is not set, then data has
        //  been read.  In this case, we want to Unpack the report into our
        //  input info and then send a message to the main thread to display
        //  the new data.
        //
        if (!pDlg->m_bTerminateThread)
        {
            UnpackReport(
                pHidDevice->InputReportBuffer,
                pHidDevice->Caps.InputReportByteLength,
                HidP_Input,
                pHidDevice->InputData,
                pHidDevice->InputDataLength,
                pHidDevice->Ppd);

            pDlg->PostMessage(WM_DISPLAY_READ_DATA);

            WaitForSingleObject(pDlg->m_displayEvent, INFINITE );
        }
    } while ( !pDlg->m_bTerminateThread && !pDlg->m_bDoOneRead );

AsyncRead_End:
    if (NULL != completionEvent) {
        CloseHandle(completionEvent);
    }

    pDlg->PostMessage(WM_READ_DONE);

    return 0;
}
