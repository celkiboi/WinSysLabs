#undef UNICODE
#undef UNICODE_
#include <windows.h>
#include <stdio.h>
#include <time.h>
#define BUF_SIZE 8192

int main(int argc, LPTSTR argv[])
{
	if (argc != 3) {
		printf("Usage: cp file1 file2\n");
		return 1;
	}

	clock_t start = clock();
	if (!CopyFile(argv[1], argv[2], FALSE)) {
		printf("CopyFile Error: %x\n", GetLastError());
		return 2;
	}

	clock_t end = clock();
	double elapsed_time_ms = (end - start) / (double)CLOCKS_PER_SEC * 1000;
	printf("Elapsed time in ms: %f\n", elapsed_time_ms);
	return 0;
}
