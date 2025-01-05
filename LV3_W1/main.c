#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <stdint.h>
#include <time.h>
#include <math.h>

#define N 768
#define ARRAY_SIZE N*N
#define THREAD_COUNT 12

typedef struct ThreadParam
{
	uint64_t beginRow;
	uint64_t endRow;
	float* matrix;
}THREAD_PARAM;

void DoMatrixWriteSingleThreaded(float* matrix)
{
	for (uint64_t i = 0; i < N; i++)
	{
		for (uint64_t j = 0; j < N; j++)
		{
			float value = 0;
			for (uint64_t k = 0; k < i; k++)
				value += k * sinf(j) - j * cosf(k);

			matrix[i * N + j] = value;
		}
	}
}

DWORD WINAPI DoMatrixWriteMultiThreaded(PVOID pThParam)
{
	THREAD_PARAM* thread_param = (THREAD_PARAM*)pThParam;
	uint64_t beginRow = thread_param->beginRow;
	uint64_t endRow = thread_param->endRow;
	float* matrix = thread_param->matrix;

	for (uint64_t i = beginRow; i < endRow; i++)
	{
		for (uint64_t j = 0; j < N; j++)
		{
			float value = 0;
			for (uint64_t k = 0; k < i; k++)
				value += k * sinf(j) - j * cosf(k);

			matrix[i * N + j] = value;
		}
	}

	ExitThread(0);
}

BOOL CheckIfMatrixIsWrittenProperly(float* matrix)
{
	for (uint64_t i = 0; i < N; i++)
	{
		for (uint64_t j = 0; j < N; j++)
		{
			float value = 0;
			for (uint64_t k = 0; k < i; k++)
				value += k * sinf(j) - j * cosf(k);

			if (matrix[i * N + j] != value)
				return FALSE;
		}
	}

	return TRUE;
}

int main(void)
{
	float* matrix = (float*)malloc(ARRAY_SIZE * sizeof(float));
	if (matrix == NULL)
		exit(-1);

	time_t time = clock();
	DoMatrixWriteSingleThreaded(matrix);
	time = clock() - time;
	puts("SINGLE THREAD RESULTS: ");
	printf("Time to write: %zu ms\n", time);

	if (CheckIfMatrixIsWrittenProperly(matrix))
		puts("Matrix contains right values");
	else
		puts("Matrix contains wrong values");

	free(matrix);

	matrix = (float*)malloc(ARRAY_SIZE * sizeof(float));
	if (matrix == NULL)
		exit(-1);

	time = clock();

	THREAD_PARAM* thread_params = (THREAD_PARAM*)calloc(THREAD_COUNT, sizeof(THREAD_PARAM));
	if (thread_params == NULL)
		exit(-2);

	HANDLE thread_handles[THREAD_COUNT] = { NULL };

	for (uint8_t i = 0; i < THREAD_COUNT; i++)
	{
		thread_params[i].beginRow = i * (N / THREAD_COUNT);
		thread_params[i].endRow = thread_params[i].beginRow + (N / THREAD_COUNT);
		thread_params[i].matrix = matrix;

		if (i == (THREAD_COUNT - 1))
			thread_params[i].endRow += N % THREAD_COUNT;
	
		thread_handles[i] = CreateThread(NULL, 0, DoMatrixWriteMultiThreaded, &(thread_params[i]), 0, NULL);
		if (thread_handles[i] == NULL)
			exit(-4);
	}

	WaitForMultipleObjects(THREAD_COUNT, thread_handles, TRUE, INFINITE);
	time = clock() - time;

	puts("MULTI THREAD RESULTS: ");
	printf("Time to write: %zu ms\n", time);

	if (CheckIfMatrixIsWrittenProperly(matrix))
		puts("Matrix contains right values");
	else
		puts("Matrix contains wrong values");

	free(thread_params);
	free(matrix);
	return 0;
}