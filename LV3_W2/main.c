#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <stdint.h>
#include <time.h>
#include <math.h>

#define N 768
#define ARRAY_SIZE N*N
#define THREAD_COUNT 2

typedef struct ThreadParam
{
	uint8_t threadID;
	uint64_t beginCol;
	uint64_t endCol;
	float* matrix;
}THREAD_PARAM;

float RowAverageValue(float* matrix, uint64_t row)
{
	uint64_t start = row * N;
	uint64_t end = start + N;
	float value = 0;
	for (uint64_t i = start; i < end; i++)
	{
		value += matrix[i];
	}
	return value / (end - start);
}

void DoMatrixWriteSingleThreaded(float* matrix)
{
	for (uint64_t i = 0; i < N; i++)
	{
		float previousRowAverage = (i > 0) ? RowAverageValue(matrix, i - 1) : 0;
		for (uint64_t j = 0; j < N; j++)
		{
			float value = 0;
			for (uint64_t k = 0; k < i; k++)
				value += k * sinf(j) - j * cosf(k);

			matrix[i * N + j] = value + previousRowAverage;
		}
	}
}

uint64_t finishedThreadsCount = 0;
HANDLE rowCompleteEvent;

DWORD WINAPI DoMatrixWriteMultiThreaded(PVOID pThParam)
{
	THREAD_PARAM* thread_param = (THREAD_PARAM*)pThParam;
	float* matrix = thread_param->matrix;
	uint64_t beginCol = thread_param->beginCol;
	uint64_t endCol = thread_param->endCol;
	uint8_t threadID = thread_param->threadID;

	for (uint64_t i = 0; i < N; i++)
	{
		float previousRowAverage = (i > 0) ? RowAverageValue(matrix, i - 1) : 0;
		for (uint64_t j = beginCol; j < endCol; j++)
		{
			float value = 0;
			for (uint64_t k = 0; k < i; k++)
				value += k * sinf(j) - j * cosf(k);

			matrix[i * N + j] = value + previousRowAverage;
		}

		BOOL shouldReset = FALSE;
		if (InterlockedIncrement(&finishedThreadsCount) == THREAD_COUNT)
		{
			InterlockedExchange(&finishedThreadsCount, 0);
			SetEvent(rowCompleteEvent);
			shouldReset = TRUE;
		}

		WaitForSingleObject(rowCompleteEvent, INFINITE);

		if (shouldReset)
			ResetEvent(rowCompleteEvent);
	}

	ExitThread(0);
}

BOOL CheckIfMatrixIsWrittenProperly(float* matrix)
{
	for (uint64_t i = 0; i < N; i++)
	{
		float previousRowAverage = (i > 0) ? RowAverageValue(matrix, i - 1) : 0;
		for (uint64_t j = 0; j < N; j++)
		{
			float value = 0;
			for (uint64_t k = 0; k < i; k++)
				value += k * sinf(j) - j * cosf(k);

			if (matrix[i * N + j] != (value + previousRowAverage))
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
	
	rowCompleteEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (rowCompleteEvent == NULL)
		exit(-3);

	HANDLE thread_handles[THREAD_COUNT] = { NULL };

	for (uint8_t i = 0; i < THREAD_COUNT; i++)
	{
		thread_params[i].beginCol = i * (N / THREAD_COUNT);
		thread_params[i].endCol = thread_params[i].beginCol + (N / THREAD_COUNT);
		thread_params[i].threadID = i;
		thread_params[i].matrix = matrix;

		if (i == (THREAD_COUNT - 1))
			thread_params[i].endCol += N % THREAD_COUNT;

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

	for (uint8_t i = 0; i < THREAD_COUNT; i++)
		CloseHandle(thread_handles[i]);
	CloseHandle(rowCompleteEvent);
	free(thread_params);
	free(matrix);
	return 0;
}