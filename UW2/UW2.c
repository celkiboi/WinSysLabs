#include <stdio.h>
#include <errno.h>
#include <time.h>
#define BUF_SIZE 8192

int main(int argc, char* argv[])
{
	FILE* in_file, * out_file;
	char rec[BUF_SIZE];
	size_t bytes_in, bytes_out;
	if (argc != 3) {
		printf("Usage: cp file1 file2\n");
		return 1;
	}

	clock_t start = clock();
	in_file = fopen(argv[1], "rb");
	if (in_file == NULL) {
		perror(argv[1]);
		return 2;
	}
	out_file = fopen(argv[2], "wb");
	if (out_file == NULL) {
		perror(argv[2]);
		return 3;
	}

	/* Process the input file a record at a time. */

	while ((bytes_in = fread(rec, 1, BUF_SIZE, in_file)) > 0) {
		bytes_out = fwrite(rec, 1, bytes_in, out_file);
		if (bytes_out != bytes_in) {
			perror("Fatal write error.");
			return 4;
		}
	}

	fclose(in_file);
	fclose(out_file);

	clock_t end = clock();
	double elapsed_time_ms = (end - start) / (double)CLOCKS_PER_SEC * 1000;
	printf("Elapsed time in ms: %f\n", elapsed_time_ms);
	return 0;
}