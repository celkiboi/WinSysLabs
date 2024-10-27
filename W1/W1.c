#undef UNICODE
#undef UNICODE_
#include <windows.h>
#include <stdio.h>
#include <time.h>
#define BUF_SIZE 8192

int main(int argc, LPTSTR argv[])
{
	HANDLE hIn, hOut;
	DWORD nIn, nOut;
	CHAR Buffer[BUF_SIZE] = {'\0'};
	if (argc != 3) {
		printf("Usage: cp file1 file2\n");
		return 1;
	}

	clock_t start = clock();
	hIn = CreateFile(argv[1], GENERIC_READ, FILE_SHARE_READ, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hIn == INVALID_HANDLE_VALUE) {
		printf("Cannot open input file. Error: %x\n", GetLastError());
		return 2;
	}

	hOut = CreateFile(argv[2], GENERIC_WRITE, 0, NULL,
		CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hOut == INVALID_HANDLE_VALUE) {
		printf("Cannot open output file. Error: %x\n", GetLastError());
		return 3;
	}
	while (ReadFile(hIn, Buffer, BUF_SIZE, &nIn, NULL) && nIn > 0) {
		WriteFile(hOut, Buffer, nIn, &nOut, NULL);
		if (nIn != nOut) {
			printf("Fatal write error: %x\n", GetLastError());
			return 4;
		}
	}
	CloseHandle(hIn);
	CloseHandle(hOut);

	clock_t end = clock();
	double elapsed_time_ms = (end - start) / (double)CLOCKS_PER_SEC * 1000;
	printf("Elapsed time in ms: %f\n", elapsed_time_ms);
	return 0;
}
