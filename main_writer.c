// Важно: после компиляции этой программы исполняемый файл должен получить имя writer.exe (для совместимости
// с main_manager.c) 

#include "map_properties.h"

#include <stdlib.h>
#include <stdio.h>
#include <Windows.h>
#include <tchar.h>

int main(int argc, char * argv[])
{
    HANDLE hMapping = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, BUF_SIZE, _T(MAP_NAME));
	if (!hMapping) {
        printf("Writer: Can't create file mapping! Error code:%d\n", GetLastError());
        return EXIT_FAILURE;
    }

	SharedMemory * sh_data = (SharedMemory*) MapViewOfFile(hMapping, FILE_MAP_WRITE, 0, 0, 0);
	if (!sh_data) {
		printf("Writer: Can't map view of file! Error code:%d\n", GetLastError());
        return EXIT_FAILURE;
    }
	
    HANDLE can_read = OpenEvent(EVENT_MODIFY_STATE | SYNCHRONIZE, FALSE, _T("CAN_READ_EVENT"));
	if (!can_read) {
		printf("Writer: Error at openning CAN_READ_EVENT event! Error code:%d\n", GetLastError());
		return EXIT_FAILURE;
	}
	HANDLE can_write = OpenEvent(EVENT_MODIFY_STATE | SYNCHRONIZE, FALSE, _T("CAN_WRITE_EVENT"));
	if (!can_write) {
		printf("Writer: Error at openning CAN_WRITE_EVENT event! Error code:%d\n", GetLastError());
		return EXIT_FAILURE;
	}
	HANDLE mutex = OpenMutex(MUTEX_MODIFY_STATE | SYNCHRONIZE, FALSE, _T("MUTEX"));
	if (!can_write) {
		printf("Writer: Error at openning mutex! Error code:%d\n", GetLastError());
		return EXIT_FAILURE;
	}

	int process_index = atoi(argv[1]);
	while (sh_data->cur_index != process_index)
		Sleep(0);
	size_t elems_left = MESSAGE_LENGTH;
	while (elems_left)
	{
		WaitForSingleObject(can_write, INFINITE);
		WaitForSingleObject(mutex, INFINITE);
		size_t elems_to_write = min(elems_left, BUF_SIZE);
		for (int i = 0; i < elems_to_write; ++i)
			sh_data->buffer[i] = process_index;
		sh_data->written_to_buffer = elems_to_write;
		elems_left -= elems_to_write;
		ResetEvent(can_write);
		SetEvent(can_read);
		ReleaseMutex(mutex);
	}
	++sh_data->cur_index;

	UnmapViewOfFile(hMapping);
	return EXIT_SUCCESS;
}
