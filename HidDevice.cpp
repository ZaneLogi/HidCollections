#include "StdAfx.h"
#include "HidDevice.h"

#include <strsafe.h>
#include <intsafe.h>

CHidDevice::CHidDevice(void)
{
    DevicePath = NULL;
    HidDevice = INVALID_HANDLE_VALUE;
    Ppd = NULL;
    InputReportBuffer = NULL;
    InputData = NULL;
    InputButtonCaps = NULL;
    InputValueCaps = NULL;
    OutputReportBuffer = NULL;
    OutputData = NULL;
    OutputButtonCaps = NULL;
    OutputValueCaps = NULL;
    FeatureReportBuffer = NULL;
    FeatureData = NULL;
    FeatureButtonCaps = NULL;
    FeatureValueCaps = NULL;
}

CHidDevice::~CHidDevice(void)
{
    Close();
}

void CHidDevice::Close()
{
    if (NULL != DevicePath)
    {
        free(DevicePath);
        DevicePath = NULL;
    }

    if (INVALID_HANDLE_VALUE != HidDevice)
    {
        CloseHandle(HidDevice);
        HidDevice = INVALID_HANDLE_VALUE;
    }

    if (NULL != Ppd)
    {
        HidD_FreePreparsedData(Ppd);
        Ppd = NULL;
    }

    if (NULL != InputReportBuffer)
    {
        free(InputReportBuffer);
        InputReportBuffer = NULL;
    }

    if (NULL != InputData)
    {
        PHID_DATA data = InputData;
        for ( int i = 0; i < Caps.NumberInputButtonCaps; i++ )
        {
            ASSERT(data->IsButtonData);
            if ( data->IsButtonData && data->ButtonData.Usages )
            {
                free(data->ButtonData.Usages);
            }
            data++;
        }
        free(InputData);
        InputData = NULL;
    }

    if (NULL != InputButtonCaps)
    {
        free(InputButtonCaps);
        InputButtonCaps = NULL;
    }

    if (NULL != InputValueCaps)
    {
        free(InputValueCaps);
        InputValueCaps = NULL;
    }

    if (NULL != OutputReportBuffer)
    {
        free(OutputReportBuffer);
        OutputReportBuffer = NULL;
    }

    if (NULL != OutputData)
    {
        PHID_DATA data = OutputData;
        for ( int i = 0; i < Caps.NumberOutputButtonCaps; i++ )
        {
            ASSERT(data->IsButtonData);
            if ( data->IsButtonData && data->ButtonData.Usages )
            {
                free(data->ButtonData.Usages);
            }
            data++;
        }
        free(OutputData);
        OutputData = NULL;
    }

    if (NULL != OutputButtonCaps)
    {
        free(OutputButtonCaps);
        OutputButtonCaps = NULL;
    }

    if (NULL != OutputValueCaps)
    {
        free(OutputValueCaps);
        OutputValueCaps = NULL;
    }

    if (NULL != FeatureReportBuffer)
    {
        free(FeatureReportBuffer);
        FeatureReportBuffer = NULL;
    }

    if (NULL != FeatureData)
    {
        PHID_DATA data = FeatureData;
        for ( int i = 0; i < Caps.NumberFeatureButtonCaps; i++ )
        {
            ASSERT(data->IsButtonData);
            if ( data->IsButtonData && data->ButtonData.Usages )
            {
                free(data->ButtonData.Usages);
            }
            data++;
        }
        free(FeatureData);
        FeatureData = NULL;
    }

    if (NULL != FeatureButtonCaps)
    {
        free(FeatureButtonCaps);
        FeatureButtonCaps = NULL;
    }

    if (NULL != FeatureValueCaps)
    {
        free(FeatureValueCaps);
        FeatureValueCaps = NULL;
    }
}

bool CHidDevice::Open(
    LPCTSTR devicePath,
    BOOL    hasReadAccess,
    BOOL    hasWriteAccess,
    BOOL    isOverlapped,
    BOOL    isExclusive)
/*++
RoutineDescription:
    Given the HardwareDeviceInfo, representing a handle to the plug and
    play information, and deviceInfoData, representing a specific hid device,
    open that device and fill in all the relivant information in the given
    HID_DEVICE structure.

    return if the open and initialization was successfull or not.

--*/
{
    DWORD    accessFlags = 0;
    DWORD    sharingFlags = 0;
    bool    bSuccess = false;
    INT        iDevicePathSize;

    if (NULL == devicePath)
        goto exit;

    iDevicePathSize = (INT)_tcslen(devicePath) + 1;

    DevicePath = (TCHAR*)calloc(iDevicePathSize, sizeof(TCHAR));

    if (NULL == DevicePath)
        goto exit;

    StringCchCopy(DevicePath, iDevicePathSize, devicePath);

    if (hasReadAccess)
    {
        accessFlags |= GENERIC_READ;
    }

    if (hasWriteAccess)
    {
        accessFlags |= GENERIC_WRITE;
    }

    if (!isExclusive)
    {
        sharingFlags = FILE_SHARE_READ | FILE_SHARE_WRITE;
    }

    //
    //  The hid.dll api's do not pass the overlapped structure into deviceiocontrol
    //  so to use them we must have a non overlapped device.  If the request is for
    //  an overlapped device we will close the device below and get a handle to an
    //  overlapped device
    //
    HidDevice = CreateFile(
        DevicePath,
        accessFlags,
        sharingFlags,
        NULL,        // no SECURITY_ATTRIBUTES structure
        OPEN_EXISTING, // No special create flags
        0,   // Open device as non-overlapped so we can get data
        NULL);       // No template file

    if (INVALID_HANDLE_VALUE == HidDevice)
        goto exit;

    OpenedForRead = hasReadAccess;
    OpenedForWrite = hasWriteAccess;
    OpenedOverlapped = isOverlapped;
    OpenedExclusive = isExclusive;

    //
    // If the device was not opened as overlapped, then fill in the rest of the
    //  HidDevice structure.  However, if opened as overlapped, this handle cannot
    //  be used in the calls to the HidD_ exported functions since each of these
    //  functions does synchronous I/O.
    //

    if (!HidD_GetPreparsedData(HidDevice, &Ppd))
        goto exit;

    if (!HidD_GetAttributes(HidDevice, &Attributes))
        goto exit;

    if (!HidP_GetCaps(Ppd, &Caps))
        goto exit;

    //
    // At this point the client has a choice.  It may chose to look at the
    // Usage and Page of the top level collection found in the HIDP_CAPS
    // structure.  In this way it could just use the usages it knows about.
    // If either HidP_GetUsages or HidP_GetUsageValue return an error then
    // that particular usage does not exist in the report.
    // This is most likely the preferred method as the application can only
    // use usages of which it already knows.
    // In this case the app need not even call GetButtonCaps or GetValueCaps.
    //
    // In this example, however, we will call FillDeviceInfo to look for all
    //    of the usages in the device.
    //

    if (!FillDeviceInfo())
        goto exit;

    if (isOverlapped)
    {
        CloseHandle(HidDevice);

        HidDevice = CreateFile(
            DevicePath,
            accessFlags,
            sharingFlags,
            NULL,        // no SECURITY_ATTRIBUTES structure
            OPEN_EXISTING, // No special create flags
            FILE_FLAG_OVERLAPPED, // Now we open the device as overlapped
            NULL);       // No template file

        if (INVALID_HANDLE_VALUE == HidDevice)
            goto exit;
    }

    bSuccess = true;

exit:
    if (!bSuccess)
    {
        DWORD dwErr = GetLastError();
        Close();
    }
    return bSuccess;
}

bool CHidDevice::FillDeviceInfo()
{
    ULONG                numValues;
    USHORT               numCaps;
    PHIDP_BUTTON_CAPS    buttonCaps;
    PHIDP_VALUE_CAPS     valueCaps;
    PHID_DATA            data;
    ULONG                i;
    USAGE                usage;
    UINT                 dataIdx;
    ULONG                newFeatureDataLength;
    ULONG                tmpSum;

    //
    // setup Input Data buffers.
    //

    //
    // Allocate memory to hold on input report
    //
    InputReportBuffer = (PCHAR)
        calloc(Caps.InputReportByteLength, sizeof (CHAR));


    //
    // Allocate memory to hold the button and value capabilities.
    // NumberXXCaps is in terms of array elements.
    //
    InputButtonCaps = buttonCaps = (PHIDP_BUTTON_CAPS)
        calloc(Caps.NumberInputButtonCaps, sizeof (HIDP_BUTTON_CAPS));

    if (NULL == buttonCaps)
        return false; // caller frees the memory

    InputValueCaps = valueCaps = (PHIDP_VALUE_CAPS)
        calloc(Caps.NumberInputValueCaps, sizeof (HIDP_VALUE_CAPS));

    if (NULL == valueCaps)
        return false; // caller frees the memory

    //
    // Have the HidP_X functions fill in the capability structure arrays.
    //
    numCaps = Caps.NumberInputButtonCaps;

    if(numCaps > 0)
    {
        if(HIDP_STATUS_SUCCESS != (HidP_GetButtonCaps (HidP_Input,
                            buttonCaps,
                            &numCaps,
                            Ppd)))
        {
            return false;
        }
    }

    numCaps = Caps.NumberInputValueCaps;

    if(numCaps > 0)
    {
        if(HIDP_STATUS_SUCCESS != (HidP_GetValueCaps (HidP_Input,
                           valueCaps,
                           &numCaps,
                           Ppd)))
        {
            return false;
        }
    }


    //
    // Depending on the device, some value caps structures may represent more
    // than one value.  (A range).  In the interest of being verbose, over
    // efficient, we will expand these so that we have one and only one
    // struct _HID_DATA for each value.
    //
    // To do this we need to count up the total number of values are listed
    // in the value caps structure.  For each element in the array we test
    // for range if it is a range then UsageMax and UsageMin describe the
    // usages for this range INCLUSIVE.
    //

    numValues = 0;
    for (i = 0; i < Caps.NumberInputValueCaps; i++, valueCaps++)
    {
        if (valueCaps->IsRange)
        {
            numValues += valueCaps->Range.UsageMax - valueCaps->Range.UsageMin + 1;
            if(valueCaps->Range.UsageMin >= valueCaps->Range.UsageMax + Caps.NumberInputButtonCaps)
            {
                return false; // overrun check
            }
        }
        else
        {
            numValues++;
        }
    }

    valueCaps = InputValueCaps;

    //
    // Allocate a buffer to hold the struct _HID_DATA structures.
    // One element for each set of buttons, and one element for each value
    // found.
    //
    InputDataLength = Caps.NumberInputButtonCaps + numValues;

    InputData = data = (PHID_DATA)
        calloc(InputDataLength, sizeof (HID_DATA));

    if (NULL == data)
        return false; // caller frees the memory

    //
    // Fill in the button data
    //
    dataIdx = 0;
    for (i = 0;
         i < Caps.NumberInputButtonCaps;
         i++, data++, buttonCaps++, dataIdx++)
    {
        data->IsButtonData = TRUE;
        data->Status = HIDP_STATUS_SUCCESS;
        data->UsagePage = buttonCaps->UsagePage;
        if (buttonCaps->IsRange)
        {
            data->ButtonData.UsageMin = buttonCaps -> Range.UsageMin;
            data->ButtonData.UsageMax = buttonCaps -> Range.UsageMax;
        }
        else
        {
            data -> ButtonData.UsageMin = data -> ButtonData.UsageMax = buttonCaps -> NotRange.Usage;
        }

        data->ButtonData.MaxUsageLength = HidP_MaxUsageListLength (
                                                HidP_Input,
                                                buttonCaps->UsagePage,
                                                Ppd);
        data->ButtonData.Usages = (PUSAGE)
            calloc (data->ButtonData.MaxUsageLength, sizeof (USAGE));

        data->ReportID = buttonCaps -> ReportID;
    }

    //
    // Fill in the value data
    //

    for (i = 0; i < Caps.NumberInputValueCaps ; i++, valueCaps++)
    {
        if (valueCaps->IsRange)
        {
            for (usage = valueCaps->Range.UsageMin;
                 usage <= valueCaps->Range.UsageMax;
                 usage++)
            {
                if(dataIdx >= InputDataLength)
                {
                    return false; // error case
                }
                data->IsButtonData = FALSE;
                data->Status = HIDP_STATUS_SUCCESS;
                data->UsagePage = valueCaps->UsagePage;
                data->ValueData.Usage = usage;
                data->ReportID = valueCaps -> ReportID;
                data++;
                dataIdx++;
            }
        }
        else
        {
            if(dataIdx >= InputDataLength)
            {
                return false; // error case
            }
            data->IsButtonData = FALSE;
            data->Status = HIDP_STATUS_SUCCESS;
            data->UsagePage = valueCaps->UsagePage;
            data->ValueData.Usage = valueCaps->NotRange.Usage;
            data->ReportID = valueCaps -> ReportID;
            data++;
            dataIdx++;
        }
    }

    //
    // setup Output Data buffers.
    //

    OutputReportBuffer = (PCHAR)
        calloc(Caps.OutputReportByteLength, sizeof (CHAR));

    OutputButtonCaps = buttonCaps = (PHIDP_BUTTON_CAPS)
        calloc(Caps.NumberOutputButtonCaps, sizeof (HIDP_BUTTON_CAPS));

    if (NULL == buttonCaps)
        return false; // caller frees the memory

    OutputValueCaps = valueCaps = (PHIDP_VALUE_CAPS)
        calloc(Caps.NumberOutputValueCaps, sizeof (HIDP_VALUE_CAPS));

    if (NULL == valueCaps)
        return false; // caller frees the memory

    numCaps = Caps.NumberOutputButtonCaps;
    if(numCaps > 0)
    {
        if(HIDP_STATUS_SUCCESS != (HidP_GetButtonCaps (HidP_Output,
                            buttonCaps,
                            &numCaps,
                            Ppd)))
        {
            return false;
        }
    }

    numCaps = Caps.NumberOutputValueCaps;
    if(numCaps > 0)
        {
        if(HIDP_STATUS_SUCCESS != (HidP_GetValueCaps (HidP_Output,
                           valueCaps,
                           &numCaps,
                           Ppd)))
        {
            return false;
        }
    }

    numValues = 0;
    for (i = 0; i < Caps.NumberOutputValueCaps; i++, valueCaps++)
    {
        if (valueCaps->IsRange)
        {
            numValues += valueCaps->Range.UsageMax
                       - valueCaps->Range.UsageMin + 1;
        }
        else
        {
            numValues++;
        }
    }
    valueCaps = OutputValueCaps;

    OutputDataLength = Caps.NumberOutputButtonCaps + numValues;

    OutputData = data = (PHID_DATA)
       calloc(OutputDataLength, sizeof (HID_DATA));

    if (NULL == data)
        return false; // caller frees the memory

    for (i = 0;
         i < Caps.NumberOutputButtonCaps;
         i++, data++, buttonCaps++)
    {
        if (i >= OutputDataLength)
        {
            return false;
        }

        if(FAILED(ULongAdd(Caps.NumberOutputButtonCaps,
            (valueCaps->Range).UsageMax, &tmpSum)))
        {
            return false;
        }

        if((valueCaps->Range).UsageMin == tmpSum)
        {
            return false;
        }

        data->IsButtonData = TRUE;
        data->Status = HIDP_STATUS_SUCCESS;
        data->UsagePage = buttonCaps->UsagePage;

        if (buttonCaps->IsRange)
        {
            data->ButtonData.UsageMin = buttonCaps -> Range.UsageMin;
            data->ButtonData.UsageMax = buttonCaps -> Range.UsageMax;
        }
        else
        {
            data -> ButtonData.UsageMin = data -> ButtonData.UsageMax = buttonCaps -> NotRange.Usage;
        }

        data->ButtonData.MaxUsageLength = HidP_MaxUsageListLength (
                                                   HidP_Output,
                                                   buttonCaps->UsagePage,
                                                   Ppd);

        data->ButtonData.Usages = (PUSAGE)
            calloc (data->ButtonData.MaxUsageLength, sizeof (USAGE));

        data->ReportID = buttonCaps -> ReportID;
    }

    for (i = 0; i < Caps.NumberOutputValueCaps ; i++, valueCaps++)
    {
        if (valueCaps->IsRange)
        {
            for (usage = valueCaps->Range.UsageMin;
                 usage <= valueCaps->Range.UsageMax;
                 usage++)
            {
                data->IsButtonData = FALSE;
                data->Status = HIDP_STATUS_SUCCESS;
                data->UsagePage = valueCaps->UsagePage;
                data->ValueData.Usage = usage;
                data->ReportID = valueCaps -> ReportID;
                data++;
            }
        }
        else
        {
            data->IsButtonData = FALSE;
            data->Status = HIDP_STATUS_SUCCESS;
            data->UsagePage = valueCaps->UsagePage;
            data->ValueData.Usage = valueCaps->NotRange.Usage;
            data->ReportID = valueCaps -> ReportID;
            data++;
        }
    }

    //
    // setup Feature Data buffers.
    //

    FeatureReportBuffer = (PCHAR)
           calloc(Caps.FeatureReportByteLength, sizeof (CHAR));

    FeatureButtonCaps = buttonCaps = (PHIDP_BUTTON_CAPS)
        calloc(Caps.NumberFeatureButtonCaps, sizeof (HIDP_BUTTON_CAPS));

    if (NULL == buttonCaps)
        return false; // caller frees the memory

    FeatureValueCaps = valueCaps = (PHIDP_VALUE_CAPS)
        calloc(Caps.NumberFeatureValueCaps, sizeof (HIDP_VALUE_CAPS));

    if (NULL == valueCaps)
        return false; // caller frees the memory

    numCaps = Caps.NumberFeatureButtonCaps;
    if(numCaps > 0)
    {
        if(HIDP_STATUS_SUCCESS != (HidP_GetButtonCaps (HidP_Feature,
                            buttonCaps,
                            &numCaps,
                            Ppd)))
        {
            return false;
        }
    }

    numCaps = Caps.NumberFeatureValueCaps;
    if(numCaps > 0)
    {
        if(HIDP_STATUS_SUCCESS != (HidP_GetValueCaps (HidP_Feature,
                           valueCaps,
                           &numCaps,
                           Ppd)))
        {
            return false;
        }
    }

    numValues = 0;
    for (i = 0; i < Caps.NumberFeatureValueCaps; i++, valueCaps++)
    {
        if (valueCaps->IsRange)
        {
            numValues += valueCaps->Range.UsageMax
                       - valueCaps->Range.UsageMin + 1;
        }
        else
        {
            numValues++;
        }
    }
    valueCaps = FeatureValueCaps;

    if(FAILED(ULongAdd(Caps.NumberFeatureButtonCaps,
        numValues, &newFeatureDataLength)))
    {
        return false;
    }

    FeatureDataLength = newFeatureDataLength;

    FeatureData = data = (PHID_DATA)
        calloc(FeatureDataLength, sizeof (HID_DATA));

    if (NULL == data)
        return false; // caller frees the memory

    dataIdx = 0;
    for (i = 0;
         i < Caps.NumberFeatureButtonCaps;
         i++, data++, buttonCaps++, dataIdx++)
    {
        data->IsButtonData = TRUE;
        data->Status = HIDP_STATUS_SUCCESS;
        data->UsagePage = buttonCaps->UsagePage;

        if (buttonCaps->IsRange)
        {
            data->ButtonData.UsageMin = buttonCaps -> Range.UsageMin;
            data->ButtonData.UsageMax = buttonCaps -> Range.UsageMax;
        }
        else
        {
            data -> ButtonData.UsageMin = data -> ButtonData.UsageMax = buttonCaps -> NotRange.Usage;
        }

        data->ButtonData.MaxUsageLength = HidP_MaxUsageListLength (
                                                HidP_Feature,
                                                buttonCaps->UsagePage,
                                                Ppd);
        data->ButtonData.Usages = (PUSAGE)
             calloc(data->ButtonData.MaxUsageLength, sizeof (USAGE));

        data->ReportID = buttonCaps -> ReportID;
    }

    for (i = 0; i < Caps.NumberFeatureValueCaps ; i++, valueCaps++)
    {
        if (valueCaps->IsRange)
        {
            for (usage = valueCaps->Range.UsageMin;
                 usage <= valueCaps->Range.UsageMax;
                 usage++)
            {
                if(dataIdx >= (FeatureDataLength))
                {
                    return false; // error case
                }
                data->IsButtonData = FALSE;
                data->Status = HIDP_STATUS_SUCCESS;
                data->UsagePage = valueCaps->UsagePage;
                data->ValueData.Usage = usage;
                data->ReportID = valueCaps -> ReportID;
                data++;
                dataIdx++;
            }
        }
        else
        {
            if(dataIdx >= (FeatureDataLength))
            {
                return false; // error case
            }
            data->IsButtonData = FALSE;
            data->Status = HIDP_STATUS_SUCCESS;
            data->UsagePage = valueCaps->UsagePage;
            data->ValueData.Usage = valueCaps->NotRange.Usage;
            data->ReportID = valueCaps -> ReportID;
            data++;
            dataIdx++;
        }
    }

    return true;
}

BOOLEAN
UnpackReport (
   __in_bcount(ReportBufferLength)PCHAR ReportBuffer,
   IN       USHORT               ReportBufferLength,
   IN       HIDP_REPORT_TYPE     ReportType,
   IN OUT   PHID_DATA            Data,
   IN       ULONG                DataLength,
   IN       PHIDP_PREPARSED_DATA Ppd
)
/*++
Routine Description:
   Given ReportBuffer representing a report from a HID device where the first
   byte of the buffer is the report ID for the report, extract all the HID_DATA
   in the Data list from the given report.
--*/
{
    ULONG       numUsages; // Number of usages returned from GetUsages.
    ULONG       i;
    UCHAR       reportID;
    ULONG       Index;
    ULONG       nextUsage;

    reportID = ReportBuffer[0];

    for (i = 0; i < DataLength; i++, Data++)
    {
        if (reportID == Data->ReportID)
        {
            if (Data->IsButtonData)
            {
                numUsages = Data->ButtonData.MaxUsageLength;

                Data->Status = HidP_GetUsages (ReportType,
                                               Data->UsagePage,
                                               0, // All collections
                                               Data->ButtonData.Usages,
                                               &numUsages,
                                               Ppd,
                                               ReportBuffer,
                                               ReportBufferLength);


                //
                // Get usages writes the list of usages into the buffer
                // Data->ButtonData.Usages newUsage is set to the number of usages
                // written into this array.
                // A usage cannot not be defined as zero, so we'll mark a zero
                // following the list of usages to indicate the end of the list of
                // usages
                //
                // NOTE: One anomaly of the GetUsages function is the lack of ability
                //        to distinguish the data for one ButtonCaps from another
                //        if two different caps structures have the same UsagePage
                //        For instance:
                //          Caps1 has UsagePage 07 and UsageRange of 0x00 - 0x167
                //          Caps2 has UsagePage 07 and UsageRange of 0xe0 - 0xe7
                //
                //        However, calling GetUsages for each of the data structs
                //          will return the same list of usages.  It is the
                //          responsibility of the caller to set in the HID_DEVICE
                //          structure which usages actually are valid for the
                //          that structure.
                //

                /*
                // Search through the usage list and remove those that
                //    correspond to usages outside the define ranged for this
                //    data structure.
                */

                for (Index = 0, nextUsage = 0; Index < numUsages; Index++)
                {
                    if (Data -> ButtonData.UsageMin <= Data -> ButtonData.Usages[Index] &&
                            Data -> ButtonData.Usages[Index] <= Data -> ButtonData.UsageMax)
                    {
                        Data -> ButtonData.Usages[nextUsage++] = Data -> ButtonData.Usages[Index];

                    }
                }

                if (nextUsage < Data -> ButtonData.MaxUsageLength)
                {
                    Data->ButtonData.Usages[nextUsage] = 0;
                }
            }
            else
            {
                Data->Status = HidP_GetUsageValue (
                                                ReportType,
                                                Data->UsagePage,
                                                0,               // All Collections.
                                                Data->ValueData.Usage,
                                                &Data->ValueData.Value,
                                                Ppd,
                                                ReportBuffer,
                                                ReportBufferLength);

                UCHAR myUsageValue[16];
                /* zane's experiment*/
                ULONG status = HidP_GetUsageValueArray(
                    ReportType,
                    Data->UsagePage,
                    0,               // All Collections.
                    Data->ValueData.Usage,
                    (PCHAR)myUsageValue,
                    sizeof(myUsageValue),
                    Ppd,
                    ReportBuffer,
                    ReportBufferLength);

                if (HIDP_STATUS_SUCCESS != Data->Status)
                {
                    return (FALSE);
                }

                Data->Status = HidP_GetScaledUsageValue (
                                                       ReportType,
                                                       Data->UsagePage,
                                                       0, // All Collections.
                                                       Data->ValueData.Usage,
                                                       &Data->ValueData.ScaledValue,
                                                       Ppd,
                                                       ReportBuffer,
                                                       ReportBufferLength);
            }
            Data -> IsDataSet = TRUE;
        }
    }
    return (TRUE);
}

BOOLEAN
PackReport (
   __out_bcount(ReportBufferLength)PCHAR ReportBuffer,
   IN  USHORT               ReportBufferLength,
   IN  HIDP_REPORT_TYPE     ReportType,
   IN  PHID_DATA            Data,
   IN  ULONG                DataLength,
   IN  PHIDP_PREPARSED_DATA Ppd
   )
/*++
Routine Description:
   This routine takes in a list of HID_DATA structures (DATA) and builds
      in ReportBuffer the given report for all data values in the list that
      correspond to the report ID of the first item in the list.

   For every data structure in the list that has the same report ID as the first
      item in the list will be set in the report.  Every data item that is
      set will also have it's IsDataSet field marked with TRUE.

   A return value of FALSE indicates an unexpected error occurred when setting
      a given data value.  The caller should expect that assume that no values
      within the given data structure were set.

   A return value of TRUE indicates that all data values for the given report
      ID were set without error.
--*/
{
    ULONG       numUsages; // Number of usages to set for a given report.
    ULONG       i;
    ULONG       CurrReportID;

    /*
    // All report buffers that are initially sent need to be zero'd out
    */

    memset (ReportBuffer, (UCHAR) 0, ReportBufferLength);

    /*
    // Go through the data structures and set all the values that correspond to
    //   the CurrReportID which is obtained from the first data structure
    //   in the list
    */

    CurrReportID = Data -> ReportID;

    for (i = 0; i < DataLength; i++, Data++)
    {
        /*
        // There are two different ways to determine if we set the current data
        //    structure:
        //    1) Store the report ID were using and only attempt to set those
        //        data structures that correspond to the given report ID.  This
        //        example shows this implementation.
        //
        //    2) Attempt to set all of the data structures and look for the
        //        returned status value of HIDP_STATUS_INVALID_REPORT_ID.  This
        //        error code indicates that the given usage exists but has a
        //        different report ID than the report ID in the current report
        //        buffer
        */

        if (Data -> ReportID == CurrReportID)
        {
            if (Data->IsButtonData)
            {
                numUsages = Data->ButtonData.MaxUsageLength;
                Data->Status = HidP_SetUsages (ReportType,
                                               Data->UsagePage,
                                               0, // All collections
                                               Data->ButtonData.Usages,
                                               &numUsages,
                                               Ppd,
                                               ReportBuffer,
                                               ReportBufferLength);
            }
            else
            {
                Data->Status = HidP_SetUsageValue (ReportType,
                                                   Data->UsagePage,
                                                   0, // All Collections.
                                                   Data->ValueData.Usage,
                                                   Data->ValueData.Value,
                                                   Ppd,
                                                   ReportBuffer,
                                                   ReportBufferLength);
            }

            if (HIDP_STATUS_SUCCESS != Data->Status)
            {
                return FALSE;
            }
        }
    }

    /*
    // At this point, all data structures that have the same ReportID as the
    //    first one will have been set in the given report.  Time to loop
    //    through the structure again and mark all of those data structures as
    //    having been set.
    */

    for (i = 0; i < DataLength; i++, Data++)
    {
        if (CurrReportID == Data -> ReportID)
        {
            Data -> IsDataSet = TRUE;
        }
    }
    return (TRUE);
}

bool CHidDevice::Read()
/*++
RoutineDescription:
   Given a struct _HID_DEVICE, obtain a read report and unpack the values
   into the InputData array.
--*/
{
    DWORD bytesRead;

    if (!ReadFile(
        HidDevice,
        InputReportBuffer,
        Caps.InputReportByteLength,
        &bytesRead,
        NULL))
    {
        return false;
    }

    ASSERT(bytesRead == Caps.InputReportByteLength);
    if (bytesRead != Caps.InputReportByteLength)
        return false;

    return FALSE != UnpackReport(
        InputReportBuffer,
        Caps.InputReportByteLength,
        HidP_Input,
        InputData,
        InputDataLength,
        Ppd);
}

bool CHidDevice::ReadOverlapped(HANDLE hCompletionEvent)
/*++
RoutineDescription:
   Given a struct _HID_DEVICE, obtain a read report and unpack the values
   into the InputData array.
--*/
{
    static OVERLAPPED  overlap;
    DWORD       bytesRead;
    BOOL        readStatus;

    /*
    // Setup the overlap structure using the completion event passed in to
    //  to use for signalling the completion of the Read
    */

    memset(&overlap, 0, sizeof(OVERLAPPED));

    overlap.hEvent = hCompletionEvent;

    /*
    // Execute the read call saving the return code to determine how to
    //  proceed (ie. the read completed synchronously or not).
    */

    readStatus = ReadFile(
        HidDevice,
        InputReportBuffer,
        Caps.InputReportByteLength,
        &bytesRead,
        &overlap);

    /*
    // If the readStatus is FALSE, then one of two cases occurred.
    //  1) ReadFile call succeeded but the Read is an overlapped one.  Here,
    //      we should return TRUE to indicate that the Read succeeded.  However,
    //      the calling thread should be blocked on the completion event
    //      which means it won't continue until the read actually completes
    //
    //  2) The ReadFile call failed for some unknown reason...In this case,
    //      the return code will be FALSE
    */

    if (!readStatus)
    {
        return (ERROR_IO_PENDING == GetLastError());
    }

    /*
    // If readStatus is TRUE, then the ReadFile call completed synchronously,
    //   since the calling thread is probably going to wait on the completion
    //   event, signal the event so it knows it can continue.
    */

    else
    {
        SetEvent(hCompletionEvent);
        return (TRUE);
    }
}

bool CHidDevice::Write()
/*++
RoutineDescription:
   Given a struct _HID_DEVICE, take the information in the HID_DATA array
   pack it into multiple write reports and send each report to the HID device
--*/
{
    /*
    // Begin by looping through the HID_DEVICE's HID_DATA structure and setting
    //   the IsDataSet field to FALSE to indicate that each structure has
    //   not yet been set for this Write call.
    */
    PHID_DATA pData = OutputData;

    for (ULONG Index = 0; Index < OutputDataLength; Index++, pData++)
    {
        pData->IsDataSet = FALSE;
    }

    /*
    // In setting all the data in the reports, we need to pack a report buffer
    //   and call WriteFile for each report ID that is represented by the
    //   device structure.  To do so, the IsDataSet field will be used to
    //   determine if a given report field has already been set.
    */
    bool status = true;
    pData= OutputData;
    for (ULONG Index = 0; Index < OutputDataLength; Index++, pData++)
    {
        if (!pData->IsDataSet)
        {
            /*
            // Package the report for this data structure.  PackReport will
            //    set the IsDataSet fields of this structure and any other
            //    structures that it includes in the report with this structure
            */
            PackReport(
                OutputReportBuffer,
                Caps.OutputReportByteLength,
                HidP_Output,
                pData,
                OutputDataLength - Index,
                Ppd);

            /*
            // Now a report has been packaged up...Send it down to the device
            */
            DWORD  bytesWritten;
            bool writeStatus = WriteFile(
                HidDevice,
                OutputReportBuffer,
                Caps.OutputReportByteLength,
                &bytesWritten,
                NULL)
                && (bytesWritten == Caps.OutputReportByteLength);

            status = status && writeStatus;
        }
    }
    return status;
}

bool CHidDevice::SetFeature()
/*++
RoutineDescription:
Given a struct _HID_DEVICE, take the information in the HID_DATA array
pack it into multiple reports and send it to the hid device via HidD_SetFeature()
--*/
{
    PHID_DATA pData;
    ULONG     Index;
    BOOLEAN   Status;
    BOOLEAN   FeatureStatus;
    /*
    // Begin by looping through the HID_DEVICE's HID_DATA structure and setting
    //   the IsDataSet field to FALSE to indicate that each structure has
    //   not yet been set for this SetFeature() call.
    */

    pData =  FeatureData;

    for (Index = 0; Index < FeatureDataLength; Index++, pData++)
    {
        pData -> IsDataSet = FALSE;
    }

    /*
    // In setting all the data in the reports, we need to pack a report buffer
    //   and call WriteFile for each report ID that is represented by the
    //   device structure.  To do so, the IsDataSet field will be used to
    //   determine if a given report field has already been set.
    */

    Status = TRUE;

    pData = FeatureData;
    for (Index = 0; Index < FeatureDataLength; Index++, pData++)
    {
        if (!pData -> IsDataSet)
        {
            /*
            // Package the report for this data structure.  PackReport will
            //    set the IsDataSet fields of this structure and any other
            //    structures that it includes in the report with this structure
            */

            PackReport (
                FeatureReportBuffer,
                Caps.FeatureReportByteLength,
                HidP_Feature,
                pData,
                FeatureDataLength - Index,
                Ppd);

            /*
            // Now a report has been packaged up...Send it down to the device
            */

            FeatureStatus =(HidD_SetFeature (
                HidDevice,
                FeatureReportBuffer,
                Caps.FeatureReportByteLength));

            Status = FeatureStatus && Status;
        }
    }
    return (Status != FALSE);
}

bool CHidDevice::GetFeature()
/*++
RoutineDescription:
   Given a struct _HID_DEVICE, fill in the feature data structures with
   all features on the device.  May issue multiple HidD_GetFeature() calls to
   deal with multiple report IDs.
--*/
{
    ULONG     Index;
    PHID_DATA pData;
    BOOLEAN   FeatureStatus;
    BOOLEAN   Status;

    /*
    // As with writing data, the IsDataSet value in all the structures should be
    //    set to FALSE to indicate that the value has yet to have been set
    */

    pData = FeatureData;

    for (Index = 0; Index < FeatureDataLength; Index++, pData++)
    {
        pData -> IsDataSet = FALSE;
    }

    /*
    // Next, each structure in the HID_DATA buffer is filled in with a value
    //   that is retrieved from one or more calls to HidD_GetFeature.  The
    //   number of calls is equal to the number of reportIDs on the device
    */

    Status = TRUE;
    pData = FeatureData;

    for (Index = 0; Index < FeatureDataLength; Index++, pData++)
    {
        /*
        // If a value has yet to have been set for this structure, build a report
        //    buffer with its report ID as the first byte of the buffer and pass
        //    it in the HidD_GetFeature call.  Specifying the report ID in the
        //    first specifies which report is actually retrieved from the device.
        //    The rest of the buffer should be zeroed before the call
        */

        if (!pData -> IsDataSet)
        {
            memset(FeatureReportBuffer, 0x00, Caps.FeatureReportByteLength);

            FeatureReportBuffer[0] = (UCHAR) pData -> ReportID;

            FeatureStatus = HidD_GetFeature (
                HidDevice,
                FeatureReportBuffer,
                Caps.FeatureReportByteLength);

            /*
            // If the return value is TRUE, scan through the rest of the HID_DATA
            //    structures and fill whatever values we can from this report
            */


            if (FeatureStatus)
            {
                FeatureStatus = UnpackReport (
                    FeatureReportBuffer,
                    Caps.FeatureReportByteLength,
                    HidP_Feature,
                    FeatureData,
                    FeatureDataLength,
                    Ppd);
            }

            Status = Status && FeatureStatus;
        }
   }

   return (Status != FALSE);
}

bool CHidDevice::GetValueArray(
    HIDP_REPORT_TYPE reportType,
    PHIDP_VALUE_CAPS valueCaps,
    PCHAR usageValue,
    USHORT usageValueByteLength)
{
    memset(FeatureReportBuffer, 0x00, Caps.FeatureReportByteLength);
    FeatureReportBuffer[0] = valueCaps->ReportID;

    BOOLEAN status = HidD_GetFeature(
        HidDevice,
        FeatureReportBuffer,
        Caps.FeatureReportByteLength);

    if (!status)
        return false;

    ULONG valueStatus = HidP_GetUsageValueArray(
        reportType,
        valueCaps->UsagePage,
        0,
        valueCaps->NotRange.Usage,
        usageValue,
        usageValueByteLength,
        Ppd,
        FeatureReportBuffer,
        Caps.FeatureReportByteLength);

    return HIDP_STATUS_SUCCESS == valueStatus;
}

bool CHidDevice::SetValueArray(
    HIDP_REPORT_TYPE reportType,
    PHIDP_VALUE_CAPS valueCaps,
    PCHAR usageValue,
    USHORT usageValueByteLength)
{
    memset(FeatureReportBuffer, 0x00, Caps.FeatureReportByteLength);

    ULONG valueStatus = HidP_SetUsageValueArray(
        reportType,
        valueCaps->UsagePage,
        0,
        valueCaps->NotRange.Usage,
        usageValue,
        usageValueByteLength,
        Ppd,
        FeatureReportBuffer,
        Caps.FeatureReportByteLength);

    if (HIDP_STATUS_SUCCESS != valueStatus)
        return false;

    BOOLEAN status = HidD_SetFeature(
        HidDevice,
        FeatureReportBuffer,
        Caps.FeatureReportByteLength);

    return (status == TRUE);
}



bool EnumHidDevices(HIDDEVENUMPROC pEnumFunc, PVOID pContext)
{
    GUID hidGuid;
    HidD_GetHidGuid (&hidGuid);

    //
    // Open a handle to the plug and play dev node.
    //
    HDEVINFO hardwareDeviceInfo = SetupDiGetClassDevs (
        &hidGuid,
        NULL, // Define no enumerator (global)
        NULL, // Define no
        (DIGCF_PRESENT | // Only Devices present
        DIGCF_DEVICEINTERFACE)); // Function class devices.

    if (INVALID_HANDLE_VALUE == hardwareDeviceInfo) {
        return false;
    }

    for (int i = 0; ; i++) {
        SP_DEVICE_INTERFACE_DATA deviceInfoData;
        deviceInfoData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

        if (SetupDiEnumDeviceInterfaces(
            hardwareDeviceInfo,
            0, // No care about specific PDOs
            &hidGuid,
            i,
            &deviceInfoData)) {
                ULONG predictedLength = 0;
                ULONG requiredLength = 0;
                //
                // allocate a function class device data structure to receive the
                // goods about this particular device.
                //
                SetupDiGetDeviceInterfaceDetail(
                    hardwareDeviceInfo,
                    &deviceInfoData,
                    NULL, // probing so no output buffer yet
                    0, // probing so output buffer length of zero
                    &requiredLength,
                    NULL); // not interested in the specific dev-node

                predictedLength = requiredLength;

                PSP_DEVICE_INTERFACE_DETAIL_DATA functionClassDeviceData =
                    (PSP_DEVICE_INTERFACE_DETAIL_DATA)malloc(predictedLength);
                if (functionClassDeviceData) {
                    functionClassDeviceData->cbSize = sizeof (SP_DEVICE_INTERFACE_DETAIL_DATA);
                    ZeroMemory(functionClassDeviceData->DevicePath, sizeof(functionClassDeviceData->DevicePath));
                } else {
                    SetupDiDestroyDeviceInfoList(hardwareDeviceInfo);
                    return false;
                }

                SP_DEVINFO_DATA did = {sizeof(SP_DEVINFO_DATA)};

                //
                // Retrieve the information from Plug and Play.
                //
                if (!SetupDiGetDeviceInterfaceDetail(
                    hardwareDeviceInfo,
                    &deviceInfoData,
                    functionClassDeviceData,
                    predictedLength,
                    &requiredLength,
                    &did))
                {
                    SetupDiDestroyDeviceInfoList(hardwareDeviceInfo);
                    free(functionClassDeviceData);
                    return false;
                }

                TCHAR deviceName[256];
                if (!SetupDiGetDeviceRegistryProperty(hardwareDeviceInfo, &did,
                    SPDRP_FRIENDLYNAME, NULL, (PBYTE)deviceName, sizeof(deviceName), NULL)) {
                        if (!SetupDiGetDeviceRegistryProperty(hardwareDeviceInfo, &did,
                            SPDRP_DEVICEDESC, NULL, (PBYTE)deviceName, sizeof(deviceName), NULL)) {
                                _tcscpy_s(deviceName,_T("NONAME"));
                        }
                }

                //
                // Notify the caller
                //
                BOOL b = pEnumFunc(deviceName, functionClassDeviceData->DevicePath, pContext);
                free(functionClassDeviceData);
                if (!b)
                    break; // stop enumeration process
        } else {
            if (ERROR_NO_MORE_ITEMS == GetLastError()) {
                break;
            }
        }
    }

    SetupDiDestroyDeviceInfoList(hardwareDeviceInfo);
    return true;
}

bool GetHidDeviceName(LPCTSTR pszDevPath, LPTSTR pszDevName, int nDevNameSize)
{
    pszDevName[0] = NULL;

    GUID hidGuid;
    HidD_GetHidGuid (&hidGuid);

    //
    // Open a handle to the plug and play dev node.
    //
    HDEVINFO hardwareDeviceInfo = SetupDiGetClassDevs (
        &hidGuid,
        NULL, // Define no enumerator (global)
        NULL, // Define no
        (DIGCF_PRESENT | // Only Devices present
        DIGCF_DEVICEINTERFACE)); // Function class devices.

    if (INVALID_HANDLE_VALUE == hardwareDeviceInfo) {
        return false;
    }

    for (int i = 0; ; i++) {
        SP_DEVICE_INTERFACE_DATA deviceInfoData;
        deviceInfoData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

        if (SetupDiEnumDeviceInterfaces(
            hardwareDeviceInfo,
            0, // No care about specific PDOs
            &hidGuid,
            i,
            &deviceInfoData)) {
                ULONG predictedLength = 0;
                ULONG requiredLength = 0;
                //
                // allocate a function class device data structure to receive the
                // goods about this particular device.
                //
                SetupDiGetDeviceInterfaceDetail(
                    hardwareDeviceInfo,
                    &deviceInfoData,
                    NULL, // probing so no output buffer yet
                    0, // probing so output buffer length of zero
                    &requiredLength,
                    NULL); // not interested in the specific dev-node

                predictedLength = requiredLength;

                PSP_DEVICE_INTERFACE_DETAIL_DATA functionClassDeviceData =
                    (PSP_DEVICE_INTERFACE_DETAIL_DATA)malloc(predictedLength);
                if (functionClassDeviceData) {
                    functionClassDeviceData->cbSize = sizeof (SP_DEVICE_INTERFACE_DETAIL_DATA);
                    ZeroMemory(functionClassDeviceData->DevicePath, sizeof(functionClassDeviceData->DevicePath));
                } else {
                    SetupDiDestroyDeviceInfoList(hardwareDeviceInfo);
                    return false;
                }

                SP_DEVINFO_DATA did = {sizeof(SP_DEVINFO_DATA)};

                //
                // Retrieve the information from Plug and Play.
                //
                if (!SetupDiGetDeviceInterfaceDetail(
                    hardwareDeviceInfo,
                    &deviceInfoData,
                    functionClassDeviceData,
                    predictedLength,
                    &requiredLength,
                    &did))
                {
                    SetupDiDestroyDeviceInfoList(hardwareDeviceInfo);
                    free(functionClassDeviceData);
                    return false;
                }

                int nRet = _tcsicmp(functionClassDeviceData->DevicePath, pszDevPath);
                free(functionClassDeviceData);
                if (nRet != 0)
                    continue;

                // found the device
                if (!SetupDiGetDeviceRegistryProperty(hardwareDeviceInfo, &did,
                    SPDRP_FRIENDLYNAME, NULL, (PBYTE)pszDevName, nDevNameSize, NULL)) {
                        if (!SetupDiGetDeviceRegistryProperty(hardwareDeviceInfo, &did,
                            SPDRP_DEVICEDESC, NULL, (PBYTE)pszDevName, nDevNameSize, NULL)) {
                                _tcscpy_s(pszDevName, nDevNameSize, _T("NONAME"));
                        }
                }
                break;
        } else {
            if (ERROR_NO_MORE_ITEMS == GetLastError()) {
                break;
            }
        }
    }

    SetupDiDestroyDeviceInfoList(hardwareDeviceInfo);
    return true;
}

VOID
ReportToString(
   PHID_DATA pData,
   __inout_bcount(iBuffSize) LPTSTR szBuff,
   UINT      iBuffSize
)
{
    PTCHAR  pszWalk;
    PUSAGE  pUsage;
    ULONG   i;
    UINT    iRemainingBuffer;
    UINT    iStringLength;
    UINT    j;

    //
    // For button data, all the usages in the usage list are to be displayed
    //

    if (pData -> IsButtonData)
    {
        if(FAILED(StringCchPrintf (szBuff,
                        iBuffSize,
                        _T("Usage Page: 0x%x, Usages: "),
                        pData -> UsagePage)))
        {
            for(j=0; j<sizeof(szBuff); j++)
            {
                szBuff[j] = '\0';
            }
            return;  // error case
        }

        iRemainingBuffer = 0;
        iStringLength = (UINT)_tcslen(szBuff);
        pszWalk = szBuff + iStringLength;
        if (iStringLength < iBuffSize)
        {
            iRemainingBuffer = iBuffSize - iStringLength;
        }

        for (i = 0, pUsage = pData -> ButtonData.Usages;
                     i < pData -> ButtonData.MaxUsageLength;
                         i++, pUsage++)
        {
            if (0 == *pUsage)
            {
                break; // A usage of zero is a non button.
            }
            if(FAILED(StringCchPrintf (pszWalk, iRemainingBuffer, _T(" 0x%x"), *pUsage)))
            {
                return; // error case
            }
            iRemainingBuffer -= (UINT)_tcslen(pszWalk);
            pszWalk += _tcslen(pszWalk);
        }
    }
    else
    {
        if(FAILED(StringCchPrintf (szBuff,
                        iBuffSize,
                        _T("Usage Page: 0x%x, Usage: 0x%x, Scaled: %d, Value: %d"),
                        pData->UsagePage,
                        pData->ValueData.Usage,
                        pData->ValueData.ScaledValue,
                        pData->ValueData.Value)))
        {
            return; // error case
        }
    }
}

INT
PrepareDataFields(
    PHID_DATA           pData,
    ULONG               ulDataLength,
    PWRITE_DATA_STRUCT  pWriteData,
    int                 iMaxElements
)
{
    INT i;
    PHID_DATA pWalk;

    pWalk = pData;

    for (i = 0; (i < iMaxElements) && ((unsigned) i < ulDataLength); i++)
    {
        if (!pWalk->IsButtonData)
        {
            StringCbPrintf(pWriteData[i].szLabel,
                           MAX_PATH,
                           _T("ValueCap; ReportID: 0x%x, UsagePage=0x%x, Usage=0x%x"),
                           pWalk->ReportID,
                           pWalk->UsagePage,
                           pWalk->ValueData.Usage);
        }
        else
        {
            StringCbPrintf(pWriteData[i].szLabel,
                           MAX_PATH,
                           _T("Button; ReportID: 0x%x, UsagePage=0x%x, UsageMin: 0x%x, UsageMax: 0x%x"),
                           pWalk->ReportID,
                           pWalk->UsagePage,
                           pWalk->ButtonData.UsageMin,
                           pWalk->ButtonData.UsageMax);
        }
        pWalk++;
     }
     return i;
}  //end function iPrepareDataFields//

BOOL
ParseData(
    PHID_DATA           pData,
    PWRITE_DATA_STRUCT  pWriteData,
    int                 iCount,
    int                 *piErrorLine
)
{
    INT       iCap;
    PHID_DATA pWalk;
    BOOL      noError = TRUE;

    pWalk = pData;

    for (iCap = 0; (iCap < iCount) && noError; iCap++)
    {
        //
        // Check to see if our data is a value cap or not
        //

        if (!pWalk->IsButtonData)
        {
            pWalk -> ValueData.Value = _tstol(pWriteData[iCap].szValue);
        }
        else
        {
            if (!SetButtonUsages(pWalk, pWriteData[iCap].szValue) )
            {
               *piErrorLine = iCap;

               noError = FALSE;
            }
        }
        pWalk++;
    }
    return (noError);
}

BOOL
SetButtonUsages(
    PHID_DATA   pCap,
    _In_ LPTSTR pszInputString
)
{
    TCHAR   szTempString[MAX_PATH];
    TCHAR   pszDelimiter[] = _T(" ");
    TCHAR   *pszContext = NULL;
    PTCHAR  pszToken;
    INT     iLoop;
    PUSAGE  pUsageWalk;
    BOOL    bNoError=TRUE;

    if(FAILED(StringCbCopy(szTempString, MAX_PATH, pszInputString) ))
    {
        return FALSE;
    }

#pragma warning(push)
#pragma prefast(disable:28193, "pszToken and pUsageWalk are examined")

    pszToken = _tcstok_s(szTempString, pszDelimiter, &pszContext);

    pUsageWalk = pCap -> ButtonData.Usages;

    memset(pUsageWalk, 0, pCap->ButtonData.MaxUsageLength * sizeof(USAGE));

    for (iLoop = 0; ((ULONG) iLoop < pCap->ButtonData.MaxUsageLength) && (pszToken != NULL) && bNoError; iLoop++)
    {
        *pUsageWalk = (USAGE) _tstol(pszToken);

        pszToken = _tcstok_s(NULL, pszDelimiter, &pszContext);

        pUsageWalk++;
    }
#pragma warning(pop)

     return bNoError;
} //end function bSetButtonUsages//