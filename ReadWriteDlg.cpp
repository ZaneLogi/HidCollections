// CReadWriteDlg.cpp : implementation file
//

#include "stdafx.h"
#include "HidCollections.h"
#include "ReadWriteDlg.h"
#include "afxdialogex.h"
#include "HexString.h"

#define WM_READ_DONE (WM_USER+100)
#define WM_DISPLAY_READ_DATA (WM_USER+101)

// CReadWriteDlg dialog

IMPLEMENT_DYNAMIC(CReadWriteDlg, CDialogEx)

CReadWriteDlg::CReadWriteDlg(CHidDevice* pHidDevice, bool modeless, CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_READ_WRITE, pParent)
{
	m_pHidDevice = pHidDevice;
    m_bModeless = modeless;
	m_readThread = NULL;
	m_displayEvent = NULL;
	m_bTerminateThread = false;
}

CReadWriteDlg::~CReadWriteDlg()
{
}

void CReadWriteDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, m_ecOutput);
	DDX_Control(pDX, IDC_EDIT2, m_ecWriteData);
}


BEGIN_MESSAGE_MAP(CReadWriteDlg, CDialogEx)
    ON_WM_MOVE()
    ON_WM_SIZE()
    ON_WM_GETMINMAXINFO()
    ON_MESSAGE(WM_READ_DONE, &CReadWriteDlg::OnReadDone)
    ON_MESSAGE(WM_DISPLAY_READ_DATA, &CReadWriteDlg::OnDisplayData)
    ON_BN_CLICKED(IDOK, &CReadWriteDlg::OnBnClickedOk)
    ON_BN_CLICKED(IDCANCEL, &CReadWriteDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_BTN_SEND, &CReadWriteDlg::OnBnClickedBtnSend)
    ON_WM_NCDESTROY()
END_MESSAGE_MAP()


// CReadWriteDlg message handlers

BOOL CReadWriteDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  Add extra initialization here

	BEGIN_OBJ_MAP(CReadWriteDlg);
	OBJ_DEFINE_SCALEABLE(IDC_EDIT1);
	OBJ_DEFINE_BOTTOM(IDC_EDIT2);
	OBJ_DEFINE_BOTTOM_RIGHT(IDC_BTN_SEND);
	OBJ_DEFINE_BOTTOM_RIGHT(IDOK);
	END_OBJ_MAP();

	VERIFY(m_editFont.CreateFont(
		40,                        // nHeight
		0,                         // nWidth
		0,                         // nEscapement
		0,                         // nOrientation
		FW_NORMAL,                 // nWeight
		FALSE,                     // bItalic
		FALSE,                     // bUnderline
		0,                         // cStrikeOut
		ANSI_CHARSET,              // nCharSet
		OUT_DEFAULT_PRECIS,        // nOutPrecision
		CLIP_DEFAULT_PRECIS,       // nClipPrecision
		DEFAULT_QUALITY,           // nQuality
		DEFAULT_PITCH | FF_SWISS,  // nPitchAndFamily
		_T("Courier")));             // lpszFacename

	m_ecOutput.SetFont(&m_editFont);
	m_ecWriteData.SetFont(&m_editFont);

    CString strItem;
    strItem.Format(_T("%s, Device %d, UsagePage 0%x, Usage 0%x"),
        m_pHidDevice->DeviceName(),
        HandleToULong(m_pHidDevice->HidDevice),
        m_pHidDevice->Caps.UsagePage,
        m_pHidDevice->Caps.Usage);
    SetWindowText(strItem);

	m_displayEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	auto result = m_asyncDevice.Open(m_pHidDevice->DevicePath,
		TRUE, // read access
		FALSE, // write access
		TRUE, // overlapped
		FALSE); // exclusive

	if (!result) {
		MessageBox(_T("Unable to open device for asynchronous reading"),
			_T("Application Error"), MB_ICONEXCLAMATION);
	}

    ReadAsync();

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

void CReadWriteDlg::OnMove(int x, int y)
{
	SAVE_WINDOW_PLACEMENT();
}

void CReadWriteDlg::OnSize(UINT nType, int cx, int cy)
{
	__super::OnSize(nType, cx, cy); // need to change to CDialogEx if the base is CDialogEx
	UPDATE_OBJ_POSITION(cx, cy);
	SAVE_WINDOW_PLACEMENT();
}

void CReadWriteDlg::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
	UPDATE_MINMAX_INFO(lpMMI);
	__super::OnGetMinMaxInfo(lpMMI); // need to change to CDialogEx if the base is CDialogEx
}

void CReadWriteDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
    if (m_bModeless)
    {
        ShowWindow(SW_HIDE);
        // let the parent window destroy this modeless dialog
        return;
    }

    m_bTerminateThread = true;
    WaitForSingleObject(m_readThread, INFINITE);

    CloseHandle(m_displayEvent);
    m_asyncDevice.Close();

	CDialogEx::OnOK();
}

void CReadWriteDlg::OnBnClickedCancel()
{
    OnBnClickedOk();
}

void CReadWriteDlg::OutputText(LPCTSTR format, ...)
{
	va_list args;
	va_start(args, format);

	int nBuf;
	TCHAR szBuffer[512];

	nBuf = _vstprintf_s(szBuffer, format, args);
	ASSERT(nBuf < (sizeof(szBuffer) / sizeof(szBuffer[0])));
	va_end(args);

	auto len = m_ecOutput.GetWindowTextLength();
	m_ecOutput.SetSel(len, len, TRUE);
	m_ecOutput.ReplaceSel(szBuffer);
}

void CReadWriteDlg::OnBnClickedBtnSend()
{
    CString dataString;
    m_ecWriteData.GetWindowText(dataString);

    std::vector<UCHAR> writeData;
    HexStringToArray(dataString, writeData);

    if (writeData.size() <= 0)
        return;

    CHidDevice writeDevice;
    bool status = writeDevice.Open(m_pHidDevice->DevicePath,
        FALSE, // read access
        TRUE, // write access
        FALSE, // overlapped
        FALSE); // exclusive

    if (!status)
    {
        MessageBox(_T("Couldn't open device for write access"),
            _T("Application Error"), MB_ICONEXCLAMATION);
    }
    else
    {
        writeDevice.Write(writeData.data(), writeData.size());
        writeDevice.Close();
    }
}

LRESULT CReadWriteDlg::OnReadDone(WPARAM wParam, LPARAM lParam)
{
    OutputText(_T("ReadThread stops.\r\n"));
    m_readThread = NULL;
	return 0;
}

LRESULT CReadWriteDlg::OnDisplayData(WPARAM wParam, LPARAM lParam)
{
    CHidDevice* pHidDevice = &m_asyncDevice;
    CString s = HexArrayToString((UCHAR*)pHidDevice->InputReportBuffer,
        pHidDevice->Caps.InputReportByteLength);
    OutputText(_T("%s\r\n"), (LPCTSTR)s);
	SetEvent(m_displayEvent);
	return 0;
}

void CReadWriteDlg::ReadAsync()
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
        }
    }
    else {
        //
        // Signal the terminate thread variable and
        //  wait for the read thread to complete.
        //
        m_bTerminateThread = true;
        WaitForSingleObject(m_readThread, INFINITE);
    }
}

DWORD WINAPI
CReadWriteDlg::AsynchReadThreadProc(PVOID pContext)
{
    const int READ_THREAD_TIMEOUT = 1000;

    CReadWriteDlg* pDlg = (CReadWriteDlg*)pContext;
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
            waitStatus = WaitForSingleObject(completionEvent, READ_THREAD_TIMEOUT);

            //
            // If completionEvent was signaled, then a read just completed
            //   so let's leave this loop and process the data
            //
            if (WAIT_OBJECT_0 == waitStatus) {
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

            WaitForSingleObject(pDlg->m_displayEvent, INFINITE);
        }
    } while (!pDlg->m_bTerminateThread);

AsyncRead_End:
    if (NULL != completionEvent) {
        CloseHandle(completionEvent);
    }

    pDlg->PostMessage(WM_READ_DONE);

    return 0;
}

void CReadWriteDlg::OnNcDestroy()
{
    CDialogEx::OnNcDestroy();

    // TODO: Add your message handler code here
    if (m_bModeless)
    {
        delete this;
    }
}
