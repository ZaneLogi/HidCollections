// FeatureDlg.cpp : implementation file
//

#include "stdafx.h"
#include "HidCollections.h"
#include "FeatureDlg.h"
#include "GetDataDlg.h"
#include "afxdialogex.h"
#include "HexString.h"

#define MAX_LB_ITEMS 250

// CFeatureDlg dialog

IMPLEMENT_DYNAMIC(CFeatureDlg, CDialogEx)

CFeatureDlg::CFeatureDlg(CHidDevice* pHidDevice, PHIDP_VALUE_CAPS valueCaps, CWnd* pParent /*=NULL*/)
    : CDialogEx(CFeatureDlg::IDD, pParent)
{
    m_pHidDevice = pHidDevice;
    m_pValueCaps = valueCaps;
}

CFeatureDlg::~CFeatureDlg()
{
}

void CFeatureDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LB_OUTPUT, m_lbOutput);
}


BEGIN_MESSAGE_MAP(CFeatureDlg, CDialogEx)
    ON_BN_CLICKED(IDC_BTN_READ, &CFeatureDlg::OnBnClickedBtnRead)
    ON_BN_CLICKED(IDC_BTN_WRITE, &CFeatureDlg::OnBnClickedBtnWrite)
END_MESSAGE_MAP()


// CFeatureDlg message handlers

void CFeatureDlg::OnBnClickedBtnRead()
{
    m_pHidDevice->GetFeature();

    PHID_DATA pData = m_pHidDevice->FeatureData;

    m_lbOutput.AddString(_T("------------ Read Features ---------------"));
    if (m_lbOutput.GetCount() > MAX_LB_ITEMS) {
        m_lbOutput.DeleteString(0);
    }

    for (ULONG i = 0; i < m_pHidDevice->FeatureDataLength; i++) {
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

    if (m_pValueCaps == NULL)
        return;

    //
    //
    //
    int size = m_pValueCaps->BitSize * m_pValueCaps->ReportCount;
    size = (size + 7) / 8;

    if (size > 1) {
        PCHAR pBuffer = new CHAR[size];

        bool b = m_pHidDevice->GetValueArray(HidP_Feature, m_pValueCaps, pBuffer, size);
        if (b) {
            CString tmp;
            int offset = 0;
            PCHAR p = pBuffer;
            while (size > 0) {
                CString s = HexArrayToString((PUCHAR)p, size > 16 ? 16 : size);
                tmp.Format(_T("%08X| %s"), offset, s);

                int n = m_lbOutput.AddString(tmp);
                m_lbOutput.SetCurSel(n);
                if (m_lbOutput.GetCount() > MAX_LB_ITEMS) {
                    m_lbOutput.DeleteString(0);
                }

                offset += 16;
                p += 16;
                size -= 16;
            }
            delete [] pBuffer;
        }
    }
}

void CFeatureDlg::OnBnClickedBtnWrite()
{
    const int MAX_OUTPUT_ELEMENTS = 100;
    static WRITE_DATA_STRUCT WriteData[MAX_OUTPUT_ELEMENTS];

    int count = PrepareDataFields(
        m_pHidDevice->FeatureData,
        m_pHidDevice->FeatureDataLength,
        WriteData,
        MAX_OUTPUT_ELEMENTS);

    CGetDataDlg dlg(_T("Modify features"), WriteData, count);
    if (dlg.DoModal() == IDOK) {
        INT iErrorLine = -1;
        if (!ParseData(m_pHidDevice->FeatureData, WriteData, count, &iErrorLine)) {
            CString str;
            str.Format(_T("Unable to parse line %x of output data"), iErrorLine);
            MessageBox( str, _T("Application Error"), MB_ICONEXCLAMATION);
        } else {
            int n = 0;
            if (m_pHidDevice->SetFeature()) {
                 n = m_lbOutput.AddString(_T("------------ Write Feature ---------------"));
            } else {
                n = m_lbOutput.AddString(_T("------------ Write Feature Error ---------------"));
            }

            m_lbOutput.SetCurSel(n);
            if (m_lbOutput.GetCount() > MAX_LB_ITEMS) {
                m_lbOutput.DeleteString(0);
            }
        }
    }
}
