#include <fltKernel.h>
#include <dontuse.h>
#include <suppress.h>
#include <ntstrsafe.h>


#pragma prefast(disable:__WARNING_ENCODE_MEMBER_FUNCTION_POINTER, "Not valid for kernel mode drivers")

PFLT_FILTER gFilterHandle;
/*************************************************************************
    Prototypes
*************************************************************************/

DRIVER_INITIALIZE DriverEntry;
NTSTATUS
DriverEntry (
    _In_ PDRIVER_OBJECT DriverObject,
    _In_ PUNICODE_STRING RegistryPath
    );

NTSTATUS
FsFilterUnload (
    _In_ FLT_FILTER_UNLOAD_FLAGS Flags
    );

FLT_PREOP_CALLBACK_STATUS
FsFilterPreCreateOperation (
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _Flt_CompletionContext_Outptr_ PVOID *CompletionContext
    );

FLT_POSTOP_CALLBACK_STATUS
FsFilterPostCreateOperation (
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_opt_ PVOID CompletionContext,
    _In_ FLT_POST_OPERATION_FLAGS Flags
    );

FLT_PREOP_CALLBACK_STATUS
FsFilterPreSetOperation(
_Inout_ PFLT_CALLBACK_DATA Data,
_In_ PCFLT_RELATED_OBJECTS FltObjects,
_Flt_CompletionContext_Outptr_ PVOID *CompletionContext
);

FLT_PREOP_CALLBACK_STATUS
FsFilterPreDirectoryControOperation(
_Inout_ PFLT_CALLBACK_DATA Data,
_In_ PCFLT_RELATED_OBJECTS FltObjects,
_Flt_CompletionContext_Outptr_ PVOID *CompletionContext
);

FLT_POSTOP_CALLBACK_STATUS
FsFilterPostDirectoryControOperation(
_Inout_ PFLT_CALLBACK_DATA Data,
_In_ PCFLT_RELATED_OBJECTS FltObjects,
_In_opt_ PVOID CompletionContext,
_In_ FLT_POST_OPERATION_FLAGS Flags
);


#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(PAGE, FsFilterUnload)
#endif

CONST FLT_OPERATION_REGISTRATION Callbacks[] = {
    { IRP_MJ_CREATE,
      0,
      FsFilterPreCreateOperation,
      FsFilterPostCreateOperation },

	{ IRP_MJ_SET_INFORMATION,
	  0,
	  FsFilterPreSetOperation,
	  NULL,},
	{ IRP_MJ_DIRECTORY_CONTROL,
	  0,
	  FsFilterPreDirectoryControOperation,
	  FsFilterPostDirectoryControOperation, },

    { IRP_MJ_OPERATION_END }
};

CONST FLT_REGISTRATION FilterRegistration = {

    sizeof( FLT_REGISTRATION ),         //  Size
    FLT_REGISTRATION_VERSION,           //  Version
    0,                                  //  Flags

    NULL,                               //  Context
    Callbacks,                          //  Operation callbacks

    FsFilterUnload,                     //  MiniFilterUnload

    NULL,								//  InstanceSetup
    NULL,								//  InstanceQueryTeardown
    NULL,							    //  InstanceTeardownStart
    NULL,								//  InstanceTeardownComplete

    NULL,                               //  GenerateFileName
    NULL,                               //  GenerateDestinationFileName
    NULL                                //  NormalizeNameComponent

};


/*************************************************************************
    MiniFilter initialization and unload routines.
*************************************************************************/

NTSTATUS
DriverEntry (
    _In_ PDRIVER_OBJECT DriverObject,
    _In_ PUNICODE_STRING RegistryPath
    )
{
    NTSTATUS status;

    UNREFERENCED_PARAMETER( RegistryPath );

    status = FltRegisterFilter( DriverObject,
                                &FilterRegistration,
                                &gFilterHandle );

    FLT_ASSERT( NT_SUCCESS( status ) );

    if (NT_SUCCESS( status )) {

        status = FltStartFiltering( gFilterHandle );

        if (!NT_SUCCESS( status )) {

            FltUnregisterFilter( gFilterHandle );
        }
    }

    return status;
}

NTSTATUS
FsFilterUnload (
    _In_ FLT_FILTER_UNLOAD_FLAGS Flags
    )
{
    UNREFERENCED_PARAMETER( Flags );

    PAGED_CODE();

    FltUnregisterFilter( gFilterHandle );

    return STATUS_SUCCESS;
}


/*************************************************************************
    MiniFilter callback routines.
*************************************************************************/
FLT_PREOP_CALLBACK_STATUS
FsFilterPreCreateOperation (
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _Flt_CompletionContext_Outptr_ PVOID *CompletionContext
    )
{
    UNREFERENCED_PARAMETER( FltObjects );
    UNREFERENCED_PARAMETER( CompletionContext );

	PFLT_FILE_NAME_INFORMATION FileNameInfo;
	NTSTATUS status;
	WCHAR szFileName[512 + 1] = { 0 };

	status = FltGetFileNameInformation(Data, FLT_FILE_NAME_NORMALIZED | FLT_FILE_NAME_QUERY_DEFAULT, &FileNameInfo);
	if (NT_SUCCESS(status))
	{
		status = FltParseFileNameInformation(FileNameInfo);

		if (NT_SUCCESS(status))
		{
			if (FileNameInfo->Name.MaximumLength < 260)
			{
				RtlCopyMemory(szFileName, FileNameInfo->Name.Buffer, FileNameInfo->Name.MaximumLength);
				_wcsupr(szFileName);

				if (wcsstr(szFileName, L"A.TXT") != NULL)
				{
					DbgPrint("FsFilterPreCreateOperation: %wS\n", szFileName);
					Data->IoStatus.Status = STATUS_INVALID_HANDLE;
					Data->IoStatus.Information = 0;
					FltReleaseFileNameInformation(FileNameInfo);
					return FLT_PREOP_COMPLETE;
				}
			}
		}

		FltReleaseFileNameInformation(FileNameInfo);
	}
	
	
    return FLT_PREOP_SUCCESS_WITH_CALLBACK;
}

/*************************************************************************
MiniFilter callback routines.
*************************************************************************/
FLT_PREOP_CALLBACK_STATUS
FsFilterPreDirectoryControOperation(
_Inout_ PFLT_CALLBACK_DATA Data,
_In_ PCFLT_RELATED_OBJECTS FltObjects,
_Flt_CompletionContext_Outptr_ PVOID *CompletionContext
)
{
	UNREFERENCED_PARAMETER(FltObjects);
	UNREFERENCED_PARAMETER(CompletionContext);

	/*PFLT_FILE_NAME_INFORMATION FileNameInfo;
	NTSTATUS status;
	WCHAR szFileName[512 + 1] = { 0 };
	PFILE_BOTH_DIR_INFORMATION	stDirInfo;
	PFILE_BOTH_DIR_INFORMATION	dir_info;
	UNICODE_STRING	EntryName;

	status = FltGetFileNameInformation(Data, FLT_FILE_NAME_NORMALIZED | FLT_FILE_NAME_QUERY_DEFAULT, &FileNameInfo);
	if (NT_SUCCESS(status))
	{
		status = FltParseFileNameInformation(FileNameInfo);

		if (NT_SUCCESS(status))
		{
			if (FileNameInfo->Name.MaximumLength < 260)
			{
				RtlCopyMemory(szFileName, FileNameInfo->Name.Buffer, FileNameInfo->Name.MaximumLength);
				_wcsupr(szFileName);

				DbgPrint("FsFilterPreDirectoryControOperation: %wS\n", szFileName);
			}
		}

		FltReleaseFileNameInformation(FileNameInfo);
	}*/

	return FLT_PREOP_SUCCESS_WITH_CALLBACK;
}

FLT_POSTOP_CALLBACK_STATUS
FsFilterPostDirectoryControOperation(
_Inout_ PFLT_CALLBACK_DATA Data,
_In_ PCFLT_RELATED_OBJECTS FltObjects,
_In_opt_ PVOID CompletionContext,
_In_ FLT_POST_OPERATION_FLAGS Flags
)
{
	PFLT_FILE_NAME_INFORMATION FileNameInfo;
	NTSTATUS status;
	WCHAR szFileName[512 + 1] = { 0 };
	PFILE_BOTH_DIR_INFORMATION	stDirInfo;
	PFILE_BOTH_DIR_INFORMATION	dir_info;
	UNICODE_STRING	EntryName;
	UNICODE_STRING uStrParent;
	NTSTATUS rc;
	PUNICODE_STRING puStr = NULL;
	PVOID FileInformation = NULL;
	UNICODE_STRING defaultName;
	UNICODE_STRING uStr, uStr1;
	PUNICODE_STRING nameToUse;
	PFLT_FILE_NAME_INFORMATION nameInfo = NULL;
	char * ptrParentDir = NULL;
	PCHAR ptr = NULL;
	char * buffer = NULL;
	wchar_t wcFullPathName[1024];
	int BytesReturned = 0;
	int bytesreturned = 0;
	int i = 0, iPos = 0;
	int j = 0, iLeft = 0;
	BOOLEAN bDone;

	RtlInitUnicodeString(&uStr, L"b.txt");

	status = FltGetFileNameInformation(Data, FLT_FILE_NAME_NORMALIZED | FLT_FILE_NAME_QUERY_DEFAULT, &FileNameInfo);
	if (NT_SUCCESS(status))
	{
		status = FltParseFileNameInformation(FileNameInfo);

		if (NT_SUCCESS(status))
		{
			if (FileNameInfo->Name.MaximumLength < 260)
			{
				RtlCopyMemory(szFileName, FileNameInfo->Name.Buffer, FileNameInfo->Name.MaximumLength);
				_wcsupr(szFileName);

				if (wcsstr(szFileName, L"DM") == NULL)
				{
					FltReleaseFileNameInformation(FileNameInfo);
					return FLT_POSTOP_FINISHED_PROCESSING;
				}


				DbgPrint("===========================FsFilterPostDirectoryControOperation: %wS===============\n", szFileName);
				stDirInfo = (PFILE_BOTH_DIR_INFORMATION)Data->Iopb->Parameters.DirectoryControl.QueryDirectory.DirectoryBuffer;
				ProbeForRead(&stDirInfo->FileName[0], sizeof(WCHAR), 1);
				DbgPrint("FileName : %ws\n", &stDirInfo->FileName[0]);

				do
				{
					stDirInfo = (PFILE_BOTH_DIR_INFORMATION)(((PUCHAR)stDirInfo) + stDirInfo->NextEntryOffset);
					ProbeForRead(&stDirInfo->FileName[0], sizeof(WCHAR), 1);
					DbgPrint("FileName : %ws\n", &stDirInfo->FileName[0]);

					stDirInfo->NextEntryOffset = 0;
				} while (stDirInfo->NextEntryOffset != 0);

				/*if (stDirInfo->NextEntryOffset == 0) {
					DbgPrint("NextEntryOffset = 0");
				}
				else
				{
					DbgPrint("Next offset is not zero\n");
					stDirInfo = (PFILE_BOTH_DIR_INFORMATION)(((PUCHAR)stDirInfo) + stDirInfo->NextEntryOffset);
					ProbeForRead(&stDirInfo->FileName[0], sizeof(WCHAR), 1);
					DbgPrint("2 : %ws\n", &stDirInfo->FileName[0]);

					if (stDirInfo->NextEntryOffset == 0) {
						DbgPrint("NextEntryOffset = 0");
					}
					else
					{
						DbgPrint("Next offset is not zero\n");
						stDirInfo = (PFILE_BOTH_DIR_INFORMATION)(((PUCHAR)stDirInfo) + stDirInfo->NextEntryOffset);
						ProbeForRead(&stDirInfo->FileName[0], sizeof(WCHAR), 1);
						DbgPrint("3 : %ws\n", &stDirInfo->FileName[0]);
					}
				}*/

				//bytesreturned = 0;

				//while (1) {

				//	ProbeForRead(&stDirInfo->FileName[0], sizeof(WCHAR), 1);

				//	DbgPrint("File Name : %ws\n", &stDirInfo->FileName[0]);
				//	uStr1.Length = (USHORT)stDirInfo->FileNameLength;
				//	uStr1.MaximumLength = uStr1.Length;
				//	uStr1.Buffer = &stDirInfo->FileName[0];

				//	//RtlInitUnicodeString(&uStr1,&stDirInfo->FileName[0]);

				//	/*if (RtlCompareUnicodeString(&uStr1, &uStr, TRUE) == 0)
				//	{
				//		DbgPrint("File Compared");
				//		if (stDirInfo->NextEntryOffset == 0)
				//			break;

				//		stDirInfo = (PFILE_BOTH_DIR_INFORMATION)(((PUCHAR)stDirInfo) + stDirInfo->NextEntryOffset);
				//		continue;
				//	}*/

				//	if (stDirInfo->NextEntryOffset != 0) {

				//		bytesreturned += stDirInfo->NextEntryOffset;
				//		DbgPrint("BytesRetured = %d, Offset Not Zero = %d\ n ", bytesreturned, stDirInfo->NextEntryOffset);
				//	}
				//	else {
				//		bytesreturned += sizeof(*stDirInfo) - sizeof(WCHAR)+stDirInfo->FileNameLength;
				//		DbgPrint("BytesRetured = %d, Offset = %d\ n ", bytesreturned, stDirInfo->NextEntryOffset);
				//	}
				//	//ptr+=stDirInfo->NextEntryOffset;
				//	//DbgPrint("ptr = %d\n",(ULONG)buffer);

				//	if (stDirInfo->NextEntryOffset == 0) {
				//		DbgPrint("Quit from inner loop");
				//		break;
				//	}

				//	stDirInfo = (PFILE_BOTH_DIR_INFORMATION)(((PUCHAR)stDirInfo) + stDirInfo->NextEntryOffset);
				//	DbgPrint("Next offset is not zero\n");

				//} // while end

				//if (bytesreturned > 0) {
				//	DbgPrint("BytesRetured > = %d\n", bytesreturned);
				//}
				//Data->IoStatus.Information = bytesreturned;

				//RtlCopyMemory((PVOID)Data->Iopb - > Parameters.DirectoryControl.QueryDirectory.DirectoryBuffer, (PVOID)stDirInfo, bytesreturned);
			}
		}

		FltReleaseFileNameInformation(FileNameInfo);
	}

	return FLT_POSTOP_FINISHED_PROCESSING;
}


FLT_POSTOP_CALLBACK_STATUS
FsFilterPostCreateOperation (
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_opt_ PVOID CompletionContext,
    _In_ FLT_POST_OPERATION_FLAGS Flags
    )
{
    UNREFERENCED_PARAMETER( Data );
    UNREFERENCED_PARAMETER( FltObjects );
    UNREFERENCED_PARAMETER( CompletionContext );
    UNREFERENCED_PARAMETER( Flags );

    return FLT_POSTOP_FINISHED_PROCESSING;
}


FLT_PREOP_CALLBACK_STATUS
FsFilterPreSetOperation(
_Inout_ PFLT_CALLBACK_DATA Data,
_In_ PCFLT_RELATED_OBJECTS FltObjects,
_Flt_CompletionContext_Outptr_ PVOID *CompletionContext
	)
{
	//DbgPrint("FsFilterPreSetOperation\n");

	WCHAR szFileName[512 + 1] = { 0 };

	RtlStringCchCopyNW(szFileName, 512, FltObjects->FileObject->FileName.Buffer, FltObjects->FileObject->FileName.Length);

	//DbgPrint(L"FsFilterPreSetOperation: %ws\n", szFileName);
	if (wcsstr(szFileName, L"FsFilter.sys") != NULL)
	{
		return FLT_PREOP_DISALLOW_FASTIO;
	}

	return FLT_PREOP_SUCCESS_WITH_CALLBACK;
}