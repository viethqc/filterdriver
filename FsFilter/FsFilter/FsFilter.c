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
	  NULL, },

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

	WCHAR szFileName[512 + 1] = { 0 };

	RtlStringCchCopyNW(szFileName, 512, FltObjects->FileObject->FileName.Buffer, FltObjects->FileObject->FileName.Length);

	DbgPrint("FsFilterPreCreateOperation: %s\n", szFileName);
	if (wcsstr(szFileName, L"test.txt") != NULL)
	{
		DbgPrint("File not create\n");
		return FLT_PREOP_DISALLOW_FASTIO;
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
/*
	WCHAR szFileName[512 + 1] = { 0 };

	RtlStringCchCopyNW(szFileName, 512, FltObjects->FileObject->FileName.Buffer, FltObjects->FileObject->FileName.Length);*/

	DbgPrint("FsFilterPreDirectoryControOperation\n");
	/*if (wcsstr(szFileName, L"test.txt") != NULL)
	{
		DbgPrint("File not create\n");
		return FLT_PREOP_DISALLOW_FASTIO;
	}*/

	return FLT_PREOP_SUCCESS_WITH_CALLBACK;
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
	DbgPrint("FsFilterPreSetOperation\n");

	WCHAR szFileName[512 + 1] = { 0 };

	RtlStringCchCopyNW(szFileName, 512, FltObjects->FileObject->FileName.Buffer, FltObjects->FileObject->FileName.Length);

	DbgPrint(L"FsFilterPreSetOperation: %ws\n", szFileName);
	if (wcsstr(szFileName, L"FsFilter.sys") != NULL)
	{
		return FLT_PREOP_DISALLOW_FASTIO;
	}

	return FLT_PREOP_SUCCESS_WITH_CALLBACK;
}