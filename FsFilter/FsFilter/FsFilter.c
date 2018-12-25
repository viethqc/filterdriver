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
				_wcslwr(szFileName);

				if (wcsstr(szFileName, L"a.txt") != NULL)
				{
					DbgPrint("FsFilterPreCreateOperation: %wS\n", szFileName);
					Data->IoStatus.Status = STATUS_NOT_FOUND;
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

ULONG GetNextEntryOffset(IN PVOID pData, IN FILE_INFORMATION_CLASS fileInfo)
{
	ULONG result = 0;
	if (pData == NULL)
		return result;
	switch (fileInfo)
	{
	case FileDirectoryInformation:
		result = ((PFILE_DIRECTORY_INFORMATION)pData)->NextEntryOffset;
		break;
	case FileFullDirectoryInformation:
		result = ((PFILE_FULL_DIR_INFORMATION)pData)->NextEntryOffset;
		break;
	case FileIdFullDirectoryInformation:
		result = ((PFILE_ID_FULL_DIR_INFORMATION)pData)->NextEntryOffset;
		break;
	case FileBothDirectoryInformation:
		result = ((PFILE_BOTH_DIR_INFORMATION)pData)->NextEntryOffset;
		break;
	case FileIdBothDirectoryInformation:
		result = ((PFILE_ID_BOTH_DIR_INFORMATION)pData)->NextEntryOffset;
		break;
	case FileNamesInformation:
		result = ((PFILE_NAMES_INFORMATION)pData)->NextEntryOffset;
		break;
	}
	return result;
}

VOID SetNextEntryOffset(IN PVOID pData, IN FILE_INFORMATION_CLASS fileInfo, IN ULONG Offset)
{
	if (pData == NULL)
		return;
	switch (fileInfo)
	{
	case FileDirectoryInformation:
		((PFILE_DIRECTORY_INFORMATION)pData)->NextEntryOffset = Offset;
		break;
	case FileFullDirectoryInformation:
		((PFILE_FULL_DIR_INFORMATION)pData)->NextEntryOffset = Offset;
		break;
	case FileIdFullDirectoryInformation:
		((PFILE_ID_FULL_DIR_INFORMATION)pData)->NextEntryOffset = Offset;
		break;
	case FileBothDirectoryInformation:
		((PFILE_BOTH_DIR_INFORMATION)pData)->NextEntryOffset = Offset;
		break;
	case FileIdBothDirectoryInformation:
		((PFILE_ID_BOTH_DIR_INFORMATION)pData)->NextEntryOffset = Offset;
		break;
	case FileNamesInformation:
		((PFILE_NAMES_INFORMATION)pData)->NextEntryOffset = Offset;
		break;
	}
}

PWSTR  GetEntryFileName(IN PVOID pData, IN FILE_INFORMATION_CLASS fileInfo)
{
	PWSTR  result = NULL;

	if (pData == NULL)
		return result;

	switch (fileInfo)
	{
	case FileDirectoryInformation:
		result = ((PFILE_DIRECTORY_INFORMATION)pData)->FileName;
		break;
	case FileFullDirectoryInformation:
		result = ((PFILE_FULL_DIR_INFORMATION)pData)->FileName;
		break;
	case FileIdFullDirectoryInformation:
		result = ((PFILE_ID_FULL_DIR_INFORMATION)pData)->FileName;
		break;
	case FileBothDirectoryInformation:
		result = ((PFILE_BOTH_DIR_INFORMATION)pData)->FileName;
		break;
	case FileIdBothDirectoryInformation:
		result = ((PFILE_ID_BOTH_DIR_INFORMATION)pData)->FileName;
		break;
	case FileNamesInformation:
		result = ((PFILE_NAMES_INFORMATION)pData)->FileName;
		break;
	}
	return result;
}

ULONG GetEntryFileNameLength(IN PVOID pData, IN FILE_INFORMATION_CLASS fileInfo)
{
	ULONG result = 0;

	if (pData == NULL)
		return result;

	switch (fileInfo)
	{
	case FileDirectoryInformation:
		result = ((PFILE_DIRECTORY_INFORMATION)pData)->FileNameLength;
		break;
	case FileFullDirectoryInformation:
		result = ((PFILE_FULL_DIR_INFORMATION)pData)->FileNameLength;
		break;
	case FileIdFullDirectoryInformation:
		result = ((PFILE_ID_FULL_DIR_INFORMATION)pData)->FileNameLength;
		break;
	case FileBothDirectoryInformation:
		result = ((PFILE_BOTH_DIR_INFORMATION)pData)->FileNameLength;
		break;
	case FileIdBothDirectoryInformation:
		result = ((PFILE_ID_BOTH_DIR_INFORMATION)pData)->FileNameLength;
		break;
	case FileNamesInformation:
		result = ((PFILE_NAMES_INFORMATION)pData)->FileNameLength;
		break;
	}
	return result;
}

FLT_POSTOP_CALLBACK_STATUS
FsFilterPostDirectoryControOperation(
_Inout_ PFLT_CALLBACK_DATA Data,
_In_ PCFLT_RELATED_OBJECTS FltObjects,
_In_opt_ PVOID CompletionContext,
_In_ FLT_POST_OPERATION_FLAGS Flags
)
{
	FILE_INFORMATION_CLASS fileInfo;
	WCHAR szFileName[512 + 1] = { 0 };

	NTSTATUS status = FLT_POSTOP_FINISHED_PROCESSING;

	if (!NT_SUCCESS(Data->IoStatus.Status) ||
		Data->Iopb->MinorFunction != IRP_MN_QUERY_DIRECTORY ||
		Data->Iopb->Parameters.DirectoryControl.QueryDirectory.DirectoryBuffer == NULL ||
		KeGetCurrentIrql() != PASSIVE_LEVEL
		)
	{
		return FLT_POSTOP_FINISHED_PROCESSING;
	}


	fileInfo = Data->Iopb->Parameters.DirectoryControl.QueryDirectory.FileInformationClass;
	if
		(
		fileInfo == FileDirectoryInformation ||
		fileInfo == FileFullDirectoryInformation ||
		fileInfo == FileIdFullDirectoryInformation ||
		fileInfo == FileBothDirectoryInformation ||
		fileInfo == FileIdBothDirectoryInformation ||
		fileInfo == FileNamesInformation
		)
	{

		DbgPrint("=============FsFilterPostDirectoryControOperation\n");
		PVOID prevEntry = NULL;
		PVOID curEntry = NULL;

		ULONG BufferPosition = 0;
		ULONG moveEntryOffset = 0;
		ULONG curEntryNextEntryOffset = 0;

		UNICODE_STRING curFileName;


		//get current directory buffer
		curEntry = Data->Iopb->Parameters.DirectoryControl.QueryDirectory.DirectoryBuffer;


		for (;;)
		{
			//calculate NextEntryOffset
			curEntryNextEntryOffset = GetNextEntryOffset(curEntry, fileInfo);

			//get current file name
			curFileName.Buffer = GetEntryFileName(curEntry, fileInfo);
			if (curFileName.Buffer == NULL)
				break;

			//get current file name length
			curFileName.Length = curFileName.MaximumLength = (USHORT)GetEntryFileNameLength(curEntry, fileInfo);

			RtlCopyMemory(szFileName, curFileName.Buffer, curFileName.MaximumLength);
			_wcslwr(szFileName);

			DbgPrint("FileName: %wS\n", szFileName);
			if (wcsstr(szFileName, L"b.txt") != NULL)
			{

				//if file not in the end of the buffer list
				if (curEntryNextEntryOffset > 0)
				{
					BufferPosition = ((ULONG)curEntry) - (ULONG)Data->Iopb->Parameters.DirectoryControl.QueryDirectory.DirectoryBuffer;

					//moved entry
					moveEntryOffset = (ULONG)Data->Iopb->Parameters.DirectoryControl.QueryDirectory.Length - BufferPosition - curEntryNextEntryOffset;

					//move memory
					RtlMoveMemory(curEntry, (PVOID)((PCHAR)curEntry + curEntryNextEntryOffset), (ULONG)moveEntryOffset);

					continue;
				}
				else
				{

					//if single file in directory
					if (prevEntry == NULL)
					{
						Data->IoStatus.Status = STATUS_NO_MORE_FILES;
						status = FLT_POSTOP_FINISHED_PROCESSING;
					}
					else
					{
						//if last file in the list set NextEntryOffset = 0
						SetNextEntryOffset(prevEntry, fileInfo, 0);
					}
					break;
				}
			}

			//DbgPrint("FileName: %wS\n", curFileName.Buffer);

			//end of directory buffer
			if (GetNextEntryOffset(curEntry, fileInfo) == 0)
			{
				break;
			}

			prevEntry = curEntry;

			//next file in buffer
			curEntry = (PVOID)((PCHAR)curEntry + GetNextEntryOffset(curEntry, fileInfo));
		}

	}
	return status;
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