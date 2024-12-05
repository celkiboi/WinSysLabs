#include <stdio.h>
#include <Windows.h>
#include <tchar.h>
#include <stdint.h>

#define PATH_BUFFER_SIZE 1024

// use #define DEBUG_MODE_PRINT_ALL_FILES to print all files to STD output
// use #define DEBUG_MODE_PRINT_FOLDERS to print folder to STD output

int32_t largeFilesCount = 0;
FILETIME oldestFile;

void PreorderSearch(LPTSTR path, LPTSTR searchParam)
{
	TCHAR query[PATH_BUFFER_SIZE] = { _T('\0') };
	_stprintf_s(query, sizeof(query) / sizeof(TCHAR), _T("%s%s\\"), path, searchParam);
	if (!SetCurrentDirectory(query))
	{
		_tperror(_T("Cannot change directories. Exiting..."));
		exit(-3);
	}

	WIN32_FIND_DATA fileData = { 0 };
	HANDLE searchHandle = FindFirstFile(_T("*"), &fileData);
	
	if (searchHandle == INVALID_HANDLE_VALUE)
	{
		_tperror(_T("Getting file handle error. Exiting..."));
		exit(-4);
	}

	BOOL hasOpenedFile;
	do
	{
#ifdef DEBUG_MODE_PRINT_ALL_FILES
		_tprintf(_T("%ls\n"), fileData.cFileName);
#endif
		if (_tcscmp(fileData.cFileName, _T(".")) != 0 && _tcscmp(fileData.cFileName, _T("..")) != 0)
		{
			if (fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
#ifdef DEBUG_MODE_PRINT_FOLDERS
				_tprintf(_T("%ls\n"), fileData.cFileName);
#endif
				PreorderSearch(query, fileData.cFileName);
			}
			if (CompareFileTime(&fileData.ftCreationTime, &oldestFile) < 0)
				oldestFile = fileData.ftCreationTime;
			if (fileData.nFileSizeHigh > 0)
				largeFilesCount++;
		}

		hasOpenedFile = FindNextFile(searchHandle, &fileData);
	} while (hasOpenedFile);

	FindClose(searchHandle);
}

int32_t _tmain(DWORD argc, LPTSTR argv[])
{
	if (argc < 2)
	{
		_tperror(_T("Not enough CLI arguments. Exiting..."));
		return -1;
	}

	oldestFile.dwLowDateTime = 0xFFFFFFFF;
	oldestFile.dwHighDateTime = 0xFFFFFFFF;

	/*
		TC: we have to do this magic trick because the user can enter . or ../ or any combination
		if he enters the recursive preorder function brakes
		we fix that by simply switching to that directory and getting the directory from windows
	*/
	if (!SetCurrentDirectory(argv[1]))
	{
		_tperror(_T("Cannot open the reqested path. Exiting..."));
		return -2;
	}
	TCHAR path[PATH_BUFFER_SIZE] = { _T('\0') };
	DWORD length = GetCurrentDirectory(PATH_BUFFER_SIZE, path);
	if (length == 0 || length > PATH_BUFFER_SIZE)
	{
		_tperror(_T("Cannot open the reqested path. Exiting..."));
		return -2;
	}
	PreorderSearch(path, _T("\0"));

	SYSTEMTIME oldestFileSystemTime;
	FileTimeToSystemTime(&oldestFile, &oldestFileSystemTime);
	_tprintf(_T("Subdirectory results of %ls:\nLarge files: %i\nOldest file: %i-%i-%i"), path, largeFilesCount,
		oldestFileSystemTime.wYear, oldestFileSystemTime.wMonth, oldestFileSystemTime.wDay);

	return 0;
}