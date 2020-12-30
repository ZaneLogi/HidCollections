
// HidCollectionsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "HidCollections.h"
#include "HidCollectionsDlg.h"
#include "FeatureDlg.h"
#include "ReadDataDlg.h"
#include "GetDataDlg.h"
#include "ReadWriteDlg.h"
#include "afxdialogex.h"

#include <dbt.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define INPUT_BUTTON    1
#define INPUT_VALUE     2
#define OUTPUT_BUTTON   3
#define OUTPUT_VALUE    4
#define FEATURE_BUTTON  5
#define FEATURE_VALUE   6
#define HID_CAPS        7
#define DEVICE_ATTRIBUTES 8



/*
Routine Description:
    Registers for notification of changes in the device interfaces for
    the specified interface class GUID.

Parameters:
    InterfaceClassGuid - The interface class GUID for the device
        interfaces.

    hDevNotify - Receives the device notification handle. On failure,
        this value is NULL.

Return Value:
    If the function succeeds, the return value is TRUE.
    If the function fails, the return value is FALSE.
*/
BOOL DoRegisterDeviceInterface
(
    HWND hWnd,
    GUID InterfaceClassGuid,
    HDEVNOTIFY *hDevNotify
)
{
    DEV_BROADCAST_DEVICEINTERFACE NotificationFilter;
    TCHAR szMsg[80];

    ZeroMemory( &NotificationFilter, sizeof(NotificationFilter) );
    NotificationFilter.dbcc_size =
        sizeof(DEV_BROADCAST_DEVICEINTERFACE);
    NotificationFilter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
    NotificationFilter.dbcc_classguid = InterfaceClassGuid;

    *hDevNotify = RegisterDeviceNotification( hWnd,
        &NotificationFilter,
        DEVICE_NOTIFY_WINDOW_HANDLE );

    if(!*hDevNotify)
    {
        StringCchPrintf(szMsg, sizeof(szMsg)/sizeof(szMsg[0]),
            _T("RegisterDeviceNotification failed: %d\n"),
            GetLastError());
        MessageBox(hWnd, szMsg, _T("Registration"), MB_OK);
        return FALSE;
    }

    return TRUE;
}

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
    CAboutDlg();

// Dialog Data
    enum { IDD = IDD_ABOUTBOX };

    protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
    DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnBnClickedBtnSend();
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
    ON_BN_CLICKED(IDC_BTN_SEND, &CAboutDlg::OnBnClickedBtnSend)
END_MESSAGE_MAP()


// CHidCollectionsDlg dialog




CHidCollectionsDlg::CHidCollectionsDlg(CWnd* pParent /*=NULL*/)
    : CDialogEx(CHidCollectionsDlg::IDD, pParent)
{
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CHidCollectionsDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_CB_HID_DEVICES, m_cbDeviceList);
    DDX_Control(pDX, IDC_CB_ITEMTYPE, m_cbItemType);
    DDX_Control(pDX, IDC_LB_ITEM_ATTRIBUTES, m_lbItemAttrs);
    DDX_Control(pDX, IDC_LB_ITEMS, m_lbItems);
}

BEGIN_MESSAGE_MAP(CHidCollectionsDlg, CDialogEx)
    ON_WM_MOVE()
    ON_WM_SIZE()
    ON_WM_GETMINMAXINFO()
    ON_WM_DEVICECHANGE()
    ON_WM_SYSCOMMAND()
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    ON_BN_CLICKED(IDOK, &CHidCollectionsDlg::OnBnClickedOk)
    ON_BN_CLICKED(IDCANCEL, &CHidCollectionsDlg::OnBnClickedCancel)
    ON_CBN_SELCHANGE(IDC_CB_HID_DEVICES, &CHidCollectionsDlg::OnCbnSelchangeCbHidDevices)
    ON_CBN_SELCHANGE(IDC_CB_ITEMTYPE, &CHidCollectionsDlg::OnCbnSelchangeCbItemtype)
    ON_LBN_SELCHANGE(IDC_LB_ITEMS, &CHidCollectionsDlg::OnLbnSelchangeLbItems)
    ON_BN_CLICKED(IDC_BTN_READ, &CHidCollectionsDlg::OnBnClickedBtnRead)
    ON_BN_CLICKED(IDC_BTN_FEATURE, &CHidCollectionsDlg::OnBnClickedBtnFeature)
    ON_BN_CLICKED(IDC_BTN_WRITE, &CHidCollectionsDlg::OnBnClickedBtnWrite)
    ON_BN_CLICKED(IDC_BTN_READ_WRITE, &CHidCollectionsDlg::OnBnClickedBtnReadWrite)
END_MESSAGE_MAP()


// CHidCollectionsDlg message handlers

BOOL CHidCollectionsDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    // Add "About..." menu item to system menu.

    // IDM_ABOUTBOX must be in the system command range.
    ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
    ASSERT(IDM_ABOUTBOX < 0xF000);

    CMenu* pSysMenu = GetSystemMenu(FALSE);
    if (pSysMenu != NULL)
    {
        BOOL bNameValid;
        CString strAboutMenu;
        bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
        ASSERT(bNameValid);
        if (!strAboutMenu.IsEmpty())
        {
            pSysMenu->AppendMenu(MF_SEPARATOR);
            pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
        }
    }

    // Set the icon for this dialog.  The framework does this automatically
    //  when the application's main window is not a dialog
    SetIcon(m_hIcon, TRUE);            // Set big icon
    SetIcon(m_hIcon, FALSE);        // Set small icon

    // TODO: Add extra initialization here

    // dynObj initializaition
    BEGIN_OBJ_MAP(CHidCollectionsDlg);
    OBJ_DEFINE_TOP(IDC_CB_HID_DEVICES);
    OBJ_DEFINE_TOP(IDC_CB_ITEMTYPE);
    OBJ_DEFINE_TOP(IDC_LB_ITEMS);
    OBJ_DEFINE_SCALEABLE(IDC_LB_ITEM_ATTRIBUTES);
    OBJ_DEFINE_BOTTOM_LEFT(IDC_BTN_FEATURE);
    OBJ_DEFINE_BOTTOM_LEFT(IDC_BTN_READ);
    OBJ_DEFINE_BOTTOM_LEFT(IDC_BTN_WRITE);
    OBJ_DEFINE_BOTTOM_RIGHT(IDC_BTN_READ_WRITE);
    OBJ_DEFINE_BOTTOM_RIGHT(IDOK);
    END_OBJ_MAP();

    PopulateItemTypes();
    PopulateHidDevices();

    GUID classGuid;
    HidD_GetHidGuid(&classGuid);

    DoRegisterDeviceInterface(GetSafeHwnd(), classGuid, &m_hDevNotify);

    return TRUE;  // return TRUE  unless you set the focus to a control
}

void CHidCollectionsDlg::OnMove(int x, int y)
{
    SAVE_WINDOW_PLACEMENT();
}

void CHidCollectionsDlg::OnSize(UINT nType, int cx, int cy)
{
    __super::OnSize(nType, cx, cy); // need to change to CDialogEx if the base is CDialogEx
    UPDATE_OBJ_POSITION(cx, cy);
    SAVE_WINDOW_PLACEMENT();
}

void CHidCollectionsDlg::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
    UPDATE_MINMAX_INFO(lpMMI);
    __super::OnGetMinMaxInfo(lpMMI); // need to change to CDialogEx if the base is CDialogEx
}

BOOL CHidCollectionsDlg::OnDeviceChange(UINT nEventType,DWORD_PTR dwData)
{
    PDEV_BROADCAST_HDR pdbh = (PDEV_BROADCAST_HDR)dwData;

    switch (nEventType) {
    case DBT_DEVICEARRIVAL:
        {
            if ( pdbh->dbch_devicetype != DBT_DEVTYP_DEVICEINTERFACE )
                return TRUE;

            PDEV_BROADCAST_DEVICEINTERFACE pdbdi = (PDEV_BROADCAST_DEVICEINTERFACE)dwData;

            TRACE( _T("DeviceChange: (DBT_DEVICEARRIVAL) %s\n"),
                pdbdi->dbcc_name );

            int nCount = m_cbDeviceList.GetCount();
            for (int i = 0; i < nCount; i++) {
                CHidDevice* pHidDevice = (CHidDevice*)m_cbDeviceList.GetItemData(i);
                ASSERT(pHidDevice != NULL);
                if (_tcscmp(pHidDevice->DevicePath, pdbdi->dbcc_name) == 0) {
                    // the device is existing in the list
                    return TRUE;
                }
            }

            // add to the list
            CHidDevice* pHidDevice = new CHidDevice();
            if ( pHidDevice ) {
                if (pHidDevice->Open(pdbdi->dbcc_name, FALSE, FALSE, FALSE, FALSE)) {
                    TCHAR devName[MAX_PATH];
                    GetHidDeviceName(pHidDevice->DevicePath, devName, sizeof(devName));
                    CString strItem;
                    strItem.Format( _T("%s, Device %d, UsagePage 0%x, Usage 0%x"),
                        devName,
                        HandleToULong(pHidDevice->HidDevice),
                        pHidDevice->Caps.UsagePage,
                        pHidDevice->Caps.Usage );
                    int index = m_cbDeviceList.AddString(strItem);
                    if ( index != CB_ERR ) {
                        m_cbDeviceList.SetItemData(index, (DWORD_PTR)pHidDevice);
                    } else {
                        delete pHidDevice;
                    }
                } else {
                    delete pHidDevice;
                }
            }
        }
        break;

        case DBT_DEVICEREMOVECOMPLETE:
        {
            if ( pdbh->dbch_devicetype != DBT_DEVTYP_DEVICEINTERFACE )
                return TRUE;

            PDEV_BROADCAST_DEVICEINTERFACE pdbdi = (PDEV_BROADCAST_DEVICEINTERFACE)dwData;

            TRACE( _T("DeviceChange: (DBT_DEVICEREMOVECOMPLETE) %s\n"),
                pdbdi->dbcc_name );

            CHidDevice* pTargetDevice = NULL;
            int nFoundIndex = -1;
            int nCount = m_cbDeviceList.GetCount();
            for (int i = 0; i < nCount; i++) {
                CHidDevice* pHidDevice = (CHidDevice*)m_cbDeviceList.GetItemData(i);
                ASSERT(pHidDevice != NULL);
                if (_tcsicmp(pHidDevice->DevicePath, pdbdi->dbcc_name) == 0) {
                    // the device is existing in the list
                    pTargetDevice = pHidDevice;
                    nFoundIndex = i;
                    break;
                }
            }

            if (pTargetDevice && nFoundIndex >= 0) {
                int nCurSel = m_cbDeviceList.GetCurSel();

                pTargetDevice->Close();
                m_cbDeviceList.DeleteString(nFoundIndex);

                if (nCurSel == nFoundIndex) {
                    m_lbItemAttrs.ResetContent();
                    m_lbItems.ResetContent();
                    m_cbDeviceList.SetCurSel(0);
                    OnCbnSelchangeCbHidDevices();
                }
            }
        }
        break;
    }

    return TRUE;
}

void CHidCollectionsDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
    if ((nID & 0xFFF0) == IDM_ABOUTBOX)
    {
        CAboutDlg dlgAbout;
        dlgAbout.DoModal();
    }
    else
    {
        CDialogEx::OnSysCommand(nID, lParam);
    }
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CHidCollectionsDlg::OnPaint()
{
    if (IsIconic())
    {
        CPaintDC dc(this); // device context for painting

        SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

        // Center icon in client rectangle
        int cxIcon = GetSystemMetrics(SM_CXICON);
        int cyIcon = GetSystemMetrics(SM_CYICON);
        CRect rect;
        GetClientRect(&rect);
        int x = (rect.Width() - cxIcon + 1) / 2;
        int y = (rect.Height() - cyIcon + 1) / 2;

        // Draw the icon
        dc.DrawIcon(x, y, m_hIcon);
    }
    else
    {
        CDialogEx::OnPaint();
    }
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CHidCollectionsDlg::OnQueryDragIcon()
{
    return static_cast<HCURSOR>(m_hIcon);
}

void CHidCollectionsDlg::PopulateHidDevices()
{
    EnumHidDevices(EnumDeviceProc, this);

    if ( m_cbDeviceList.GetCount() > 0 )
    {
        m_cbDeviceList.SetCurSel(0);
        OnCbnSelchangeCbHidDevices();
    }
}

BOOL CALLBACK CHidCollectionsDlg::EnumDeviceProc(LPCTSTR pszDevName, LPCTSTR pszDevPath, PVOID pContext)
{
    CHidCollectionsDlg* pDlg = (CHidCollectionsDlg*)pContext;

    CHidDevice* pHidDevice = new CHidDevice();
    if ( pHidDevice ) {
        if (pHidDevice->Open(pszDevPath, FALSE, FALSE, FALSE, FALSE)) {
            CString strItem;
            strItem.Format( _T("%s, Device %d, UsagePage 0%x, Usage 0%x"),
                pszDevName,
                HandleToULong(pHidDevice->HidDevice),
                pHidDevice->Caps.UsagePage,
                pHidDevice->Caps.Usage );
            int index = pDlg->m_cbDeviceList.AddString(strItem);
            if ( index != CB_ERR ) {
                pDlg->m_cbDeviceList.SetItemData(index, (DWORD_PTR)pHidDevice);
            } else {
                delete pHidDevice;
            }
        } else {
            delete pHidDevice;
        }
    }

    return TRUE;
}

void CHidCollectionsDlg::PopulateItemTypes()
{
    int itemIndex;
    itemIndex = m_cbItemType.AddString(_T("DEVICE ATTRIBUTES"));
    if (itemIndex != CB_ERR)
        m_cbItemType.SetItemData(itemIndex, DEVICE_ATTRIBUTES);
    itemIndex = m_cbItemType.AddString(_T("HID CAPS"));
    if (itemIndex != CB_ERR)
        m_cbItemType.SetItemData(itemIndex, HID_CAPS);
    itemIndex = m_cbItemType.AddString(_T("INPUT BUTTON"));
    if (itemIndex != CB_ERR)
        m_cbItemType.SetItemData(itemIndex, INPUT_BUTTON);
    itemIndex = m_cbItemType.AddString(_T("INPUT VALUE"));
    if (itemIndex != CB_ERR)
        m_cbItemType.SetItemData(itemIndex, INPUT_VALUE);
    itemIndex = m_cbItemType.AddString(_T("OUTPUT BUTTON"));
    if (itemIndex != CB_ERR)
        m_cbItemType.SetItemData(itemIndex, OUTPUT_BUTTON);
    itemIndex = m_cbItemType.AddString(_T("OUTPUT VALUE"));
    if (itemIndex != CB_ERR)
        m_cbItemType.SetItemData(itemIndex, OUTPUT_VALUE);
    itemIndex = m_cbItemType.AddString(_T("FEATURE BUTTON"));
    if (itemIndex != CB_ERR)
        m_cbItemType.SetItemData(itemIndex, FEATURE_BUTTON);
    itemIndex = m_cbItemType.AddString(_T("FEATURE VALUE"));
    if (itemIndex != CB_ERR)
        m_cbItemType.SetItemData(itemIndex, FEATURE_VALUE);

    m_cbItemType.SetCurSel(0);
}

void CHidCollectionsDlg::PopulateButtons(int nNumCaps, PHIDP_BUTTON_CAPS pButtonCaps, LPCTSTR prefix)
{
    CString s;
    for (int iLoop = 0; iLoop < nNumCaps; iLoop++)
    {
        s.Format(_T("%s # %d"), prefix, iLoop);
        int iIndex = m_lbItems.AddString(s);
        if (iIndex != CB_ERR)
        {
            m_lbItems.SetItemData(iIndex, (DWORD_PTR)pButtonCaps);
        }
        pButtonCaps++;
    }
    m_lbItems.SetCurSel(0);
    OnLbnSelchangeLbItems();
}

void CHidCollectionsDlg::PopulateValues(int nNumCaps, PHIDP_VALUE_CAPS pValueCaps, LPCTSTR prefix)
{
    CString s;
    for (int iLoop = 0; iLoop < nNumCaps; iLoop++)
    {
        s.Format(_T("%s # %d"), prefix, iLoop);
        int iIndex = m_lbItems.AddString(s);
        if (iIndex != CB_ERR)
        {
            m_lbItems.SetItemData(iIndex, (DWORD_PTR)pValueCaps);
        }
        pValueCaps++;
    }
    m_lbItems.SetCurSel(0);
    OnLbnSelchangeLbItems();
}

void CHidCollectionsDlg::PopulateItemAttributes(PATTR_FORMAT pFormats, PBYTE pData)
{
    CString text;
    while ( pFormats->type != VT_EMPTY )
    {
        switch (pFormats->type) {
        case VT_UI2:
            text.Format( pFormats->format, *(PUSHORT)(pData+pFormats->offset) );
            break;
        default:
            text = _T("Internal Error!!!");
            break;
        }
        m_lbItemAttrs.AddString(text);
        pFormats++;
    }
}

void CHidCollectionsDlg::PopulateButtonAttributes(PHIDP_BUTTON_CAPS pButton)
{
    CString s;

    s.Format(_T("Report ID: 0x%x"), pButton->ReportID);
    m_lbItemAttrs.AddString(s);

    s.Format(_T("Usage Page: 0x%x"), pButton->UsagePage);
    m_lbItemAttrs.AddString(s);

    s.Format(_T("Alias: %s"), pButton -> IsAlias ? _T("TRUE") : _T("FALSE"));
    m_lbItemAttrs.AddString(s);

    s.Format(_T("Link Collection: %hu"), pButton -> LinkCollection);
    m_lbItemAttrs.AddString(s);

    s.Format(_T("Link Usage Page: 0x%x"), pButton -> LinkUsagePage);
    m_lbItemAttrs.AddString(s);

    s.Format(_T("Link Usage: 0x%x"), pButton -> LinkUsage);
    m_lbItemAttrs.AddString(s);

    if (pButton->IsRange)
    {
        s.Format(_T("Usage Min: 0x%x, Usage Max: 0x%x"),
            pButton->Range.UsageMin,
            pButton->Range.UsageMax);
    }
    else
    {
        s.Format(_T("Usage: 0x%x"), pButton->NotRange.Usage);
    }
    m_lbItemAttrs.AddString(s);

    if (pButton->IsRange)
    {
         s.Format(_T("Data Index Min: 0x%x, Data Index Max: 0x%x"),
             pButton->Range.DataIndexMin,
             pButton->Range.DataIndexMax);
    }
    else
    {
         s.Format(_T("DataIndex: 0x%x"), pButton->NotRange.DataIndex);
    }
    m_lbItemAttrs.AddString(s);

    if (pButton->IsStringRange)
    {
        s.Format(_T("String Min: 0x%x, String Max: 0x%x"),
            pButton->Range.StringMin,
            pButton->Range.StringMax);
    }
    else
    {
        s.Format(_T("String Index: 0x%x"), pButton->NotRange.StringIndex);
    }
    m_lbItemAttrs.AddString(s);

    if (pButton->IsDesignatorRange)
    {
        s.Format(_T("Designator Min: 0x%x, Designator Max: 0x%x"),
            pButton->Range.DesignatorMin,
            pButton->Range.DesignatorMax);
    }
    else
    {
        s.Format(_T("Designator Index: 0x%x"), pButton->NotRange.DesignatorIndex);
    }
    m_lbItemAttrs.AddString(s);

    if (pButton->IsAbsolute)
    {
        m_lbItemAttrs.AddString(_T("Absolute: Yes"));
    }
    else
    {
        m_lbItemAttrs.AddString(_T("Absolute: No"));
    }
}

void CHidCollectionsDlg::PopulateValueAttributes(PHIDP_VALUE_CAPS pValue)
{
    CString s;

    s.Format(_T("Report ID 0x%x"), pValue->ReportID);
    m_lbItemAttrs.AddString(s);

    s.Format(_T("Usage Page: 0x%x"), pValue->UsagePage);
    m_lbItemAttrs.AddString(s);

    s.Format(_T("Bit size: 0x%x"), pValue->BitSize);
    m_lbItemAttrs.AddString(s);

    s.Format(_T("Report Count: 0x%x"), pValue->ReportCount);
    m_lbItemAttrs.AddString(s);

    s.Format(_T("Unit Exponent: 0x%x"), pValue->UnitsExp);
    m_lbItemAttrs.AddString(s);

    s.Format(_T("Has Null: 0x%x"), pValue->HasNull);
    m_lbItemAttrs.AddString(s);

    if (pValue->IsAlias)
    {
        m_lbItemAttrs.AddString(_T("Alias"));
    }
    else
    {
        m_lbItemAttrs.AddString(_T("====="));
    }

    if (pValue->IsRange)
    {
        s.Format(_T("Usage Min: 0x%x, Usage Max 0x%x"),
            pValue->Range.UsageMin,
            pValue->Range.UsageMax);
    }
    else
    {
        s.Format(_T("Usage: 0x%x"), pValue -> NotRange.Usage);
    }
    m_lbItemAttrs.AddString(s);

    if (pValue->IsRange)
    {
        s.Format(_T("Data Index Min: 0x%x, Data Index Max: 0x%x"),
            pValue->Range.DataIndexMin,
            pValue->Range.DataIndexMax);
    }
    else
    {
         s.Format(_T("DataIndex: 0x%x"), pValue->NotRange.DataIndex);
    }
    m_lbItemAttrs.AddString(s);

    s.Format(_T("Physical Minimum: %d, Physical Maximum: %d"),
        pValue->PhysicalMin,
        pValue->PhysicalMax);
    m_lbItemAttrs.AddString(s);

    s.Format(_T("Logical Minimum: 0x%x, Logical Maximum: 0x%x"),
        pValue->LogicalMin,
        pValue->LogicalMax);
    m_lbItemAttrs.AddString(s);

    if (pValue->IsStringRange)
    {
       s.Format(_T("String  Min: 0x%x String Max 0x%x"),
           pValue->Range.StringMin,
           pValue->Range.StringMax);
    }
    else
    {
        s.Format(_T("String Index: 0x%x"), pValue->NotRange.StringIndex);
    }
    m_lbItemAttrs.AddString(s);

    if (pValue->IsDesignatorRange)
    {
        s.Format(_T("Designator Minimum: 0x%x, Max: 0x%x"),
            pValue->Range.DesignatorMin,
            pValue->Range.DesignatorMax);
    }
    else
    {
        s.Format(_T("Designator Index: 0x%x"), pValue->NotRange.DesignatorIndex);
    }
    m_lbItemAttrs.AddString(s);

    if (pValue->IsAbsolute)
    {
        m_lbItemAttrs.AddString(_T("Absolute: Yes"));
    }
    else
    {
        m_lbItemAttrs.AddString(_T("Absolute: No"));
    }
}

void CHidCollectionsDlg::OnBnClickedOk()
{
    // TODO: Add your control notification handler code here
    //CDialogEx::OnOK();
    OnBnClickedCancel();
}


void CHidCollectionsDlg::OnBnClickedCancel()
{
    // TODO: Add your control notification handler code here
    int nCount = m_cbDeviceList.GetCount();
    for ( int i = 0; i < nCount; i++ )
    {
        CHidDevice* pHidDevice = (CHidDevice*)m_cbDeviceList.GetItemData(i);
        if ( pHidDevice )
        {
            pHidDevice->Close();
            delete pHidDevice;
        }
    }

    if ( m_hDevNotify )
    {
        UnregisterDeviceNotification(m_hDevNotify);
        m_hDevNotify = NULL;
    }

    CDialogEx::OnCancel();
}

void CHidCollectionsDlg::OnCbnSelchangeCbHidDevices()
{
    int itemIndex = m_cbDeviceList.GetCurSel();
    if ( itemIndex == CB_ERR )
        return;

    GetDlgItem(IDC_BTN_FEATURE)->EnableWindow(FALSE);
    GetDlgItem(IDC_BTN_READ)->EnableWindow(FALSE);
    GetDlgItem(IDC_BTN_WRITE)->EnableWindow(FALSE);

    OnCbnSelchangeCbItemtype();
}

void CHidCollectionsDlg::OnCbnSelchangeCbItemtype()
{
    m_lbItemAttrs.ResetContent();
    m_lbItems.ResetContent();

    int itemIndex = m_cbDeviceList.GetCurSel();
    if ( itemIndex == CB_ERR )
        return;

    CHidDevice* pHidDevice = (CHidDevice*)m_cbDeviceList.GetItemData(itemIndex);
    if (!pHidDevice)
        return;

    int typeIndex = m_cbItemType.GetCurSel();
    if ( typeIndex == CB_ERR )
        return;

    typeIndex = m_cbItemType.GetItemData(typeIndex);

    switch (typeIndex) {
    case DEVICE_ATTRIBUTES:
        {
            static ATTR_FORMAT formats[] =
            {
                { VT_UI2, offsetof(HIDD_ATTRIBUTES,VendorID), _T("Vendor ID: 0x%x") },
                { VT_UI2, offsetof(HIDD_ATTRIBUTES,ProductID), _T("Product ID: 0x%x") },
                { VT_UI2, offsetof(HIDD_ATTRIBUTES,VersionNumber), _T("Version Number: 0x%x") },
                { VT_EMPTY, 0 },
            };

            PopulateItemAttributes( formats, (PBYTE)&pHidDevice->Attributes );
            GetDlgItem(IDC_BTN_FEATURE)->EnableWindow(FALSE);
            GetDlgItem(IDC_BTN_READ)->EnableWindow(FALSE);
            GetDlgItem(IDC_BTN_WRITE)->EnableWindow(FALSE);
        }
        break;
    case HID_CAPS:
        {
            static ATTR_FORMAT formats[] =
            {
                { VT_UI2, offsetof(HIDP_CAPS,UsagePage),                    _T("UsagePage: 0x%x") },
                { VT_UI2, offsetof(HIDP_CAPS,Usage),                        _T("Usage: 0x%x") },
                { VT_UI2, offsetof(HIDP_CAPS,InputReportByteLength),        _T("InputReportByteLength: %d") },
                { VT_UI2, offsetof(HIDP_CAPS,OutputReportByteLength),       _T("OutputReportByteLength: %d") },
                { VT_UI2, offsetof(HIDP_CAPS,FeatureReportByteLength),      _T("FeatureReportByteLength: %d") },
                { VT_UI2, offsetof(HIDP_CAPS,NumberLinkCollectionNodes),    _T("NumberLinkCollectionNodes: %d") },
                { VT_UI2, offsetof(HIDP_CAPS,NumberInputButtonCaps),        _T("NumberInputButtonCaps: %d") },
                { VT_UI2, offsetof(HIDP_CAPS,NumberInputValueCaps),         _T("NumberInputValueCaps: %d") },
                { VT_UI2, offsetof(HIDP_CAPS,NumberInputDataIndices),       _T("NumberInputDataIndices: %d") },
                { VT_UI2, offsetof(HIDP_CAPS,NumberOutputButtonCaps),       _T("NumberOutputButtonCaps: %d") },
                { VT_UI2, offsetof(HIDP_CAPS,NumberOutputValueCaps),        _T("NumberOutputValueCaps: %d") },
                { VT_UI2, offsetof(HIDP_CAPS,NumberOutputDataIndices),      _T("NumberOutputDataIndices: %d") },
                { VT_UI2, offsetof(HIDP_CAPS,NumberFeatureButtonCaps),      _T("NumberFeatureButtonCaps: %d") },
                { VT_UI2, offsetof(HIDP_CAPS,NumberFeatureValueCaps),       _T("NumberFeatureValueCaps: %d") },
                { VT_UI2, offsetof(HIDP_CAPS,NumberFeatureDataIndices),     _T("NumberFeatureDataIndices: %d") },
                { VT_EMPTY, 0 },
            };

            PopulateItemAttributes( formats, (PBYTE)&pHidDevice->Caps );
            GetDlgItem(IDC_BTN_FEATURE)->EnableWindow(FALSE);
            GetDlgItem(IDC_BTN_READ)->EnableWindow(FALSE);
            GetDlgItem(IDC_BTN_WRITE)->EnableWindow(FALSE);
        }
        break;
    case INPUT_BUTTON:
        {
            PopulateButtons(pHidDevice->Caps.NumberInputButtonCaps,
                pHidDevice->InputButtonCaps,
                _T("Input button cap"));
            GetDlgItem(IDC_BTN_FEATURE)->EnableWindow(FALSE);
            GetDlgItem(IDC_BTN_READ)->EnableWindow(pHidDevice->Caps.NumberInputButtonCaps);
            GetDlgItem(IDC_BTN_WRITE)->EnableWindow(pHidDevice->Caps.NumberInputButtonCaps);
        }
        break;
    case OUTPUT_BUTTON:
        {
            PopulateButtons(pHidDevice->Caps.NumberOutputButtonCaps,
                pHidDevice->OutputButtonCaps,
                _T("Output button cap"));
            GetDlgItem(IDC_BTN_FEATURE)->EnableWindow(FALSE);
            GetDlgItem(IDC_BTN_READ)->EnableWindow(pHidDevice->Caps.NumberOutputButtonCaps);
            GetDlgItem(IDC_BTN_WRITE)->EnableWindow(pHidDevice->Caps.NumberOutputButtonCaps);
        }
        break;
    case FEATURE_BUTTON:
        {
            PopulateButtons(pHidDevice->Caps.NumberFeatureButtonCaps,
                pHidDevice->FeatureButtonCaps,
                _T("Feature button cap"));
            GetDlgItem(IDC_BTN_FEATURE)->EnableWindow(pHidDevice->Caps.NumberFeatureButtonCaps);
            GetDlgItem(IDC_BTN_READ)->EnableWindow(FALSE);
            GetDlgItem(IDC_BTN_WRITE)->EnableWindow(FALSE);
        }
        break;
    case INPUT_VALUE:
        {
            PopulateValues(pHidDevice->Caps.NumberInputValueCaps,
                pHidDevice->InputValueCaps,
                _T("Input value cap"));
            GetDlgItem(IDC_BTN_FEATURE)->EnableWindow(FALSE);
            GetDlgItem(IDC_BTN_READ)->EnableWindow(pHidDevice->Caps.NumberInputValueCaps);
            GetDlgItem(IDC_BTN_WRITE)->EnableWindow(pHidDevice->Caps.NumberInputValueCaps);
        }
        break;
    case OUTPUT_VALUE:
        {
            PopulateValues(pHidDevice->Caps.NumberOutputValueCaps,
                pHidDevice->OutputValueCaps,
                _T("Output value cap"));
            GetDlgItem(IDC_BTN_FEATURE)->EnableWindow(FALSE);
            GetDlgItem(IDC_BTN_READ)->EnableWindow(pHidDevice->Caps.NumberOutputValueCaps);
            GetDlgItem(IDC_BTN_WRITE)->EnableWindow(pHidDevice->Caps.NumberOutputValueCaps);
        }
        break;
    case FEATURE_VALUE:
        {
            PopulateValues(pHidDevice->Caps.NumberFeatureValueCaps,
                pHidDevice->FeatureValueCaps,
                _T("Feature value cap"));
            GetDlgItem(IDC_BTN_FEATURE)->EnableWindow(pHidDevice->Caps.NumberFeatureValueCaps);
            GetDlgItem(IDC_BTN_READ)->EnableWindow(FALSE);
            GetDlgItem(IDC_BTN_WRITE)->EnableWindow(FALSE);
        }
        break;
    }
}

void CHidCollectionsDlg::OnLbnSelchangeLbItems()
{
    m_lbItemAttrs.ResetContent();

    int itemType = m_cbItemType.GetCurSel();
    if (itemType == CB_ERR )
        return;
    itemType = m_cbItemType.GetItemData(itemType);

    int itemIndex = m_lbItems.GetCurSel();
    if (itemIndex == CB_ERR)
        return;
    DWORD_PTR itemData = m_lbItems.GetItemData(itemIndex);
    ASSERT(itemData!=NULL);

    switch (itemType) {
    case INPUT_BUTTON:
    case OUTPUT_BUTTON:
    case FEATURE_BUTTON:
        {
            PopulateButtonAttributes((PHIDP_BUTTON_CAPS)itemData);
        }
        break;
    case INPUT_VALUE:
    case OUTPUT_VALUE:
    case FEATURE_VALUE:
        {
            PopulateValueAttributes((PHIDP_VALUE_CAPS)itemData);
        }
        break;
    }
}

void CHidCollectionsDlg::OnBnClickedBtnRead()
{
    int itemIndex = m_cbDeviceList.GetCurSel();
    if ( itemIndex == CB_ERR )
        return;

    CHidDevice* pHidDevice = (CHidDevice*)m_cbDeviceList.GetItemData(itemIndex);
    if (!pHidDevice)
        return;

    CReadDataDlg dlg(pHidDevice);
    dlg.DoModal();
}


void CHidCollectionsDlg::OnBnClickedBtnFeature()
{
    int nDevIdx = m_cbDeviceList.GetCurSel();
    if (nDevIdx == CB_ERR)
        return;

    CHidDevice* pHidDevice = (CHidDevice*)m_cbDeviceList.GetItemData(nDevIdx);
    if (pHidDevice == NULL)
        return;

    PHIDP_VALUE_CAPS valueCaps = NULL;
    int itemType = m_cbItemType.GetCurSel();
    if (itemType != CB_ERR ) {
        itemType = m_cbItemType.GetItemData(itemType);

        int itemIndex = m_lbItems.GetCurSel();
        if (itemIndex != CB_ERR) {
            DWORD_PTR itemData = m_lbItems.GetItemData(itemIndex);
            ASSERT(itemData!=NULL);

            switch (itemType) {
            case INPUT_BUTTON:
            case OUTPUT_BUTTON:
            case FEATURE_BUTTON:
                {
                    //PopulateButtonAttributes((PHIDP_BUTTON_CAPS)itemData);
                }
                break;
            case INPUT_VALUE:
            case OUTPUT_VALUE:
                break;
            case FEATURE_VALUE:
                valueCaps = (PHIDP_VALUE_CAPS)itemData;
                break;
            }
        }
    }

    CFeatureDlg dlg(pHidDevice, valueCaps);
    dlg.DoModal();
}


void CHidCollectionsDlg::OnBnClickedBtnWrite()
{
    int nDevIdx = m_cbDeviceList.GetCurSel();
    if (nDevIdx == CB_ERR)
        return;

    CHidDevice* pHidDevice = (CHidDevice*)m_cbDeviceList.GetItemData(nDevIdx);
    if (pHidDevice == NULL)
        return;

    CHidDevice writeDevice;
    bool status = writeDevice.Open(pHidDevice->DevicePath,
        FALSE,
        TRUE,
        FALSE,
        FALSE);

    if (!status) {
        MessageBox(_T("Couldn't open device for write access"),
            _T("Application Error"), MB_ICONEXCLAMATION);
    } else {
        const int MAX_OUTPUT_ELEMENTS = 100;
        static WRITE_DATA_STRUCT WriteData[MAX_OUTPUT_ELEMENTS];

        int iCount = PrepareDataFields(writeDevice.OutputData,
            writeDevice.OutputDataLength,
            WriteData,
            MAX_OUTPUT_ELEMENTS);

        CGetDataDlg dlg(_T("Write to device"), WriteData, iCount);
        if (dlg.DoModal() == IDOK) {
            INT iErrorLine = -1;
            if (ParseData(writeDevice.OutputData, WriteData, iCount, &iErrorLine)) {
                writeDevice.Write();
            } else {
                CString str;
                str.Format(_T("Unable to parse line %x of output data"), iErrorLine);
                MessageBox( str, _T("Application Error"), MB_ICONEXCLAMATION);
            }
        }

        writeDevice.Close();
    }
}


void CHidCollectionsDlg::OnBnClickedBtnReadWrite()
{
    int nDevIdx = m_cbDeviceList.GetCurSel();
    if (nDevIdx == CB_ERR)
        return;

    CHidDevice* pHidDevice = (CHidDevice*)m_cbDeviceList.GetItemData(nDevIdx);
    if (pHidDevice == NULL)
        return;

    CReadWriteDlg dlg(pHidDevice);
    dlg.DoModal();
}


void CAboutDlg::OnBnClickedBtnSend()
{
    // TODO: Add your control notification handler code here
}
