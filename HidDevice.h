#pragma once



//
// A structure to hold the steady state data received from the hid device.
// Each time a read packet is received we fill in this structure.
// Each time we wish to write to a hid device we fill in this structure.
// This structure is here only for convenience.  Most real applications will
// have a more efficient way of moving the hid data to the read, write, and
// feature routines.
//
typedef struct _HID_DATA {
   BOOLEAN     IsButtonData;
   UCHAR       Reserved;
   USAGE       UsagePage;   // The usage page for which we are looking.
   ULONG       Status;      // The last status returned from the accessor function
                            // when updating this field.
   ULONG       ReportID;    // ReportID for this given data structure
   BOOLEAN     IsDataSet;   // Variable to track whether a given data structure
                            //  has already been added to a report structure

   union {
      struct {
         ULONG       UsageMin;       // Variables to track the usage minimum and max
         ULONG       UsageMax;       // If equal, then only a single usage
         ULONG       MaxUsageLength; // Usages buffer length.
         PUSAGE      Usages;         // list of usages (buttons ``down'' on the device.

      } ButtonData;
      struct {
         USAGE       Usage; // The usage describing this value;
         USHORT      Reserved;

         ULONG       Value;
         LONG        ScaledValue;
      } ValueData;
   };
} HID_DATA, *PHID_DATA;

typedef struct _HID_DEVICE {
    PTCHAR               DevicePath;
    HANDLE               HidDevice; // A file handle to the hid device.
    BOOL                 OpenedForRead;
    BOOL                 OpenedForWrite;
    BOOL                 OpenedOverlapped;
    BOOL                 OpenedExclusive;

    PHIDP_PREPARSED_DATA Ppd; // The opaque parser info describing this device
    HIDP_CAPS            Caps; // The Capabilities of this hid device.
    HIDD_ATTRIBUTES      Attributes;

    PCHAR                InputReportBuffer;
    __field_ecount(InputDataLength)
    PHID_DATA            InputData; // array of hid data structures
    ULONG                InputDataLength; // Num elements in this array.
    PHIDP_BUTTON_CAPS    InputButtonCaps;
    PHIDP_VALUE_CAPS     InputValueCaps;

    PCHAR                OutputReportBuffer;
    __field_ecount(OutputDataLength)
    PHID_DATA            OutputData;
    ULONG                OutputDataLength;
    PHIDP_BUTTON_CAPS    OutputButtonCaps;
    PHIDP_VALUE_CAPS     OutputValueCaps;

    PCHAR                FeatureReportBuffer;
    __field_ecount(FeatureDataLength)
    PHID_DATA            FeatureData;
    ULONG                FeatureDataLength;
    PHIDP_BUTTON_CAPS    FeatureButtonCaps;
    PHIDP_VALUE_CAPS     FeatureValueCaps;
} HID_DEVICE, *PHID_DEVICE;

class CHidDevice : public HID_DEVICE
{
public:
    CHidDevice(void);
    ~CHidDevice(void);

    CHidDevice(const CHidDevice&) = delete;
    CHidDevice& operator = (const CHidDevice&) = delete;

    bool Open(
        LPCTSTR devicePath,
        BOOL    hasReadAccess,
        BOOL    hasWriteAccess,
        BOOL    isOverlapped,
        BOOL    isExclusive);

    void Close();

    bool Read();
    bool ReadOverlapped(HANDLE hCompletionEvent);
    bool Write();
    bool Write(PUCHAR pData, ULONG ulSize);
    bool SetFeature();
    bool GetFeature();

    bool GetValueArray(HIDP_REPORT_TYPE reportType, PHIDP_VALUE_CAPS valueCaps, PCHAR usageValue, USHORT usageValueByteLength);
    bool SetValueArray(HIDP_REPORT_TYPE reportType, PHIDP_VALUE_CAPS valueCaps, PCHAR usageValue, USHORT usageValueByteLength);

private:
    bool FillDeviceInfo();
};

//
// Helper Function
//
typedef BOOL (CALLBACK* HIDDEVENUMPROC)(LPCTSTR strDevName, LPCTSTR strDevPath, PVOID pContext);
bool EnumHidDevices(HIDDEVENUMPROC pEnumFunc, PVOID pContext);

bool GetHidDeviceName(LPCTSTR pszDevPath, LPTSTR pszDevName, int nDevNameSize);

VOID
ReportToString(
   PHID_DATA    pData,
   __inout_bcount(iBuffSize) LPTSTR szBuff,
   UINT          iBuffSize
);

typedef struct WRITE_DATA_STRUCT
{
    TCHAR szLabel[MAX_PATH];
    TCHAR szValue[MAX_PATH];
} WRITE_DATA_STRUCT, *PWRITE_DATA_STRUCT;

int
PrepareDataFields(
    PHID_DATA pData,
    ULONG ulDataLength,
    PWRITE_DATA_STRUCT pWriteData,
    int iMaxElements
);

BOOL
ParseData(
    PHID_DATA pData,
    PWRITE_DATA_STRUCT pWriteData,
    INT iCount,
    INT *piErrorLine
);

BOOL
SetButtonUsages(
    PHID_DATA     pCap,
    _In_ LPTSTR   pszInputString
);

BOOLEAN
UnpackReport (
   __in_bcount(ReportBufferLength)PCHAR ReportBuffer,
   IN       USHORT               ReportBufferLength,
   IN       HIDP_REPORT_TYPE     ReportType,
   IN OUT   PHID_DATA            Data,
   IN       ULONG                DataLength,
   IN       PHIDP_PREPARSED_DATA Ppd
);

BOOLEAN
PackReport (
   __out_bcount(ReportBufferLength)PCHAR ReportBuffer,
   IN  USHORT               ReportBufferLength,
   IN  HIDP_REPORT_TYPE     ReportType,
   IN  PHID_DATA            Data,
   IN  ULONG                DataLength,
   IN  PHIDP_PREPARSED_DATA Ppd
);