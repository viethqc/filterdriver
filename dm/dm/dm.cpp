// dm.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "dm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// The one and only application object

CWinApp theApp;

using namespace std;

int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
	int nRetCode = 0;

	HMODULE hModule = ::GetModuleHandle(NULL);

	if (hModule != NULL)
	{
		// initialize MFC and print and error on failure
		if (!AfxWinInit(hModule, NULL, ::GetCommandLine(), 0))
		{
			// TODO: change error code to suit your needs
			_tprintf(_T("Fatal Error: MFC initialization failed\n"));
			nRetCode = 1;
		}
		else
		{
			 //TODO: code your application's behavior here.
			if (PathFileExists(L"C:\\a.txt") == FALSE)
			{
				_tprintf(_T("File not found\n"));
			}

			DWORD ftyp = GetFileAttributes(L"C:\\a.txt");
			if (ftyp == INVALID_FILE_ATTRIBUTES)
			{
				_tprintf(_T("Get file attribute error\n"));
			}

			HANDLE hFind = INVALID_HANDLE_VALUE;
			WIN32_FIND_DATA find_data;
			BOOL bFind;

			hFind = FindFirstFile(_T("C:\\dm\\*.*"), &find_data);
			if (hFind == INVALID_HANDLE_VALUE)
			{
				_tprintf(_T("Find first file error\n"));
				return 0;
			}

			_tprintf(_T("%s\n"), find_data.cFileName);
			while (bFind = FindNextFile(hFind, &find_data))
			{
				_tprintf(_T("%s\n"), find_data.cFileName);
			}
		}
	}
	else
	{
		// TODO: change error code to suit your needs
		_tprintf(_T("Fatal Error: GetModuleHandle failed\n"));
		nRetCode = 1;
	}

	return nRetCode;
}
