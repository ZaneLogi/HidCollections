#pragma once

#include "DynObj.h"
#include "HexEdit.h"
#include "HidDevice.h"

// CReadWriteDlg dialog

class CReadWriteDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CReadWriteDlg)

public:
	CReadWriteDlg(CHidDevice* pHidDevice, CWnd* pParent = nullptr);   // standard constructor
	virtual ~CReadWriteDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_READ_WRITE };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedOk();
	afx_msg LRESULT OnDisplayData(WPARAM wParam = 0, LPARAM lParam = 0);
	afx_msg LRESULT OnReadDone(WPARAM wParam = 0, LPARAM lParam = 0);

	DECLARE_OBJ_MAP();
	afx_msg void OnMove(int, int);  // ON_WM_MOVE()
	afx_msg void OnSize(UINT, int, int);    // ON_WM_SIZE()
	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);    // ON_WM_GETMINMAXINFO()

private:
	void OutputText(LPCTSTR format, ...);
	void ReadAsync();
	static DWORD WINAPI AsynchReadThreadProc(PVOID pContext);

private:
	CHidDevice* m_pHidDevice;
	HANDLE      m_readThread;
	HANDLE      m_displayEvent;
	CHidDevice  m_asyncDevice;
	bool        m_bTerminateThread;

	CFont m_editFont;
	CEdit m_ecOutput;
	CHexEdit m_ecWriteData;
public:
	afx_msg void OnBnClickedBtnSend();
};
