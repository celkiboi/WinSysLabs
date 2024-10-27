#include <Windows.h>
#ifdef _WINDOWS_
#include <io.h>
#else
#include <unistd.h>
#define _open open
#define _close close
#define _read read
#define _write write
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>
#define BUF_SIZE 8192

int main(int argc, char* argv[])
{
	int input_fd, output_fd;
	int bytes_in, bytes_out;
	char rec[BUF_SIZE];
	if (argc != 3) {
		printf("Usage: cp file1 file2\n");
		return 1;
	}

	clock_t start = clock();

	input_fd = _open(argv[1], O_RDONLY);
	if (input_fd == -1) {
		perror(argv[1]);
		return 2;
	}
	output_fd = _open(argv[2], O_WRONLY | O_CREAT, 0666);
	if (output_fd == -1) {
		perror(argv[2]);
		return 3;
	}

	/* Process the input file a record at a time. */

	while ((bytes_in = _read(input_fd, &rec, BUF_SIZE)) > 0) {
		bytes_out = _write(output_fd, &rec, (unsigned int)bytes_in);
		if (bytes_out != bytes_in) {
			perror("Fatal write error.");
			return 4;
		}
	}
	_close(input_fd);
	_close(output_fd);
	clock_t end = clock();
	double elapsed_time_ms = (end - start) / (double)CLOCKS_PER_SEC * 1000;
	printf("Elapsed time in ms: %f\n", elapsed_time_ms);
	return 0;
}