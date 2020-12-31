
// HidCollectionsDlg.h : header file
//

#pragma once
#include "afxcmn.h"
#include "afxwin.h"

#include "DynObj.h"
#include "HidDevice.h"
#include "ReadWriteDlg.h"

#include <map>

typedef struct _ATTR_FORMAT
{
    VARTYPE type;
    int offset;
    const TCHAR* format;
} ATTR_FORMAT, *PATTR_FORMAT;

// CHidCollectionsDlg dialog
class CHidCollectionsDlg : public CDialogEx
{
    static BOOL CALLBACK EnumDeviceProc(LPCTSTR pszDevName, LPCTSTR pszDevPath, PVOID pContext);

    HDEVNOTIFY m_hDevNotify;

// Construction
public:
    CHidCollectionsDlg(CWnd* pParent = NULL);    // standard constructor

// Dialog Data
    enum { IDD = IDD_HIDCOLLECTIONS_DIALOG };

    protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support


// Implementation
protected:
    HICON m_hIcon;

    // Generated message map functions
    virtual BOOL OnInitDialog();
    afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
    afx_msg void OnPaint();
    afx_msg HCURSOR OnQueryDragIcon();
    DECLARE_MESSAGE_MAP()

    DECLARE_OBJ_MAP();
    afx_msg void OnMove(int, int);  // ON_WM_MOVE()
    afx_msg void OnSize(UINT, int, int);    // ON_WM_SIZE()
    afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);    // ON_WM_GETMINMAXINFO()

    afx_msg void OnBnClickedOk();
    afx_msg void OnBnClickedCancel();
    afx_msg void OnCbnSelchangeCbHidDevices();
    afx_msg void OnCbnSelchangeCbItemtype();
    afx_msg void OnLbnSelchangeLbItems();
    afx_msg void OnBnClickedBtnRead();
    afx_msg void OnBnClickedBtnFeature();
    afx_msg void OnBnClickedBtnWrite();
    afx_msg void OnBnClickedBtnReadWrite();

    afx_msg BOOL OnDeviceChange(UINT nEventType,DWORD_PTR dwData);

    void PopulateHidDevices();
    void PopulateItemTypes();
    void PopulateButtons(int nNumCaps, PHIDP_BUTTON_CAPS pButtonCaps, LPCTSTR prefix);
    void PopulateValues(int nNumCaps, PHIDP_VALUE_CAPS pValueCaps, LPCTSTR prefix);
    void PopulateItemAttributes(PATTR_FORMAT pFormats, PBYTE pData);
    void PopulateButtonAttributes(PHIDP_BUTTON_CAPS pCaps);
    void PopulateValueAttributes(PHIDP_VALUE_CAPS pValueCaps);

private:
    CComboBox   m_cbDeviceList;
    CComboBox   m_cbItemType;
    CListBox    m_lbItems;
    CListBox    m_lbItemAttrs;

    std::map<CHidDevice*, CReadWriteDlg*> m_dlgList;
};
