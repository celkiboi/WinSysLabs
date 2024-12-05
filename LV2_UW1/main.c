#include <stdio.h>
#include <Windows.h>
#include <tchar.h>
#include <stdint.h>

// USE #define DEBUG_MODE to enable debugging CLI debugging

int32_t _tmain(DWORD argc, LPTSTR argv[])
{
	if (argc < 2)
	{
		_tperror(_T("Not enough CLI arguments. Exiting..."));
		return -1;
	}
	
	SetCurrentDirectory(argv[1]);

	WIN32_FIND_DATA fileData = { 0 };
	HANDLE searchHandle = FindFirstFile(_T("*"), &fileData);

	if (searchHandle == INVALID_HANDLE_VALUE)
	{
		_tperror(_T("Getting file handle error. Exiting..."));
		goto ErrorExit;
	}

	uint32_t filesCount = 0;
	uint32_t directoryCount = 0;
	BOOL hasOpenedFile;
	do
	{
#ifdef DEBUG_MODE
		_tprintf(_T("%ls\n"), fileData.cFileName);
#endif

		if (_tcscmp(fileData.cFileName, _T(".")) != 0 && _tcscmp(fileData.cFileName, _T("..")) != 0)
		{
			if (fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				directoryCount++;
			else
				filesCount++;
		}

		hasOpenedFile = FindNextFile(searchHandle, &fileData);
	} while (hasOpenedFile);

	_tprintf(_T("Directory \"%ls\" info:\nFiles: %i\nDirectories: %i"), argv[1], filesCount, directoryCount);

	ErrorExit:
		FindClose(searchHandle);
		return 0;
}