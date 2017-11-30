// Важно: данная программа должна быть запущена после main_manager.c, т.к. в последней инициализируетя общая память,
// необходимая для работы данной программы.

#include "map_properties.h"

#include <stdio.h>
#include <Windows.h>
#include <tchar.h>
#include <string.h>

int main()
{
    HANDLE hMapping = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, BUF_SIZE, _T(MAP_NAME));
	if (!hMapping) {
        printf("Reader: Can't create file mapping! Error code:%d\n", GetLastError());
        return EXIT_FAILURE;
    }

	SharedMemory * sh_data = (SharedMemory*) MapViewOfFile(hMapping, FILE_MAP_WRITE, 0, 0, 0);
	if (!sh_data) {
		printf("Reader: Can't map view of file! Error code:%d\n", GetLastError());
        return EXIT_FAILURE;
    }

    HANDLE can_read = OpenEvent(EVENT_MODIFY_STATE | SYNCHRONIZE, FALSE, _T("CAN_READ_EVENT"));
	if (!can_read) {
		printf("Reader: Error at openning CAN_READ_EVENT event! Error code:%d\n", GetLastError());
		return EXIT_FAILURE;
	}
	HANDLE can_write = OpenEvent(EVENT_MODIFY_STATE | SYNCHRONIZE, FALSE, _T("CAN_WRITE_EVENT"));
	if (!can_write) {
		printf("Reader: Error at openning CAN_WRITE_EVENT event! Error code:%d\n", GetLastError());
		return EXIT_FAILURE;
	}
	HANDLE mutex = OpenMutex(MUTEX_MODIFY_STATE | SYNCHRONIZE, FALSE, _T("MUTEX"));
	if (!can_write) {
		printf("Reader: Error at openning mutex! Error code:%d\n", GetLastError());
		return EXIT_FAILURE;
	}

    printf("Data read: ");
    while (sh_data->cur_index <= sh_data->max_index) {
        WaitForSingleObject(can_read, INFINITE);
        WaitForSingleObject(mutex, INFINITE);
        for (int i = 0; i < sh_data->written_to_buffer; ++i)
            printf("%d", sh_data->buffer[i]);
        sh_data->written_to_buffer = 0;
        ResetEvent(can_read);
        SetEvent(can_write);
        ReleaseMutex(mutex);
    }
    UnmapViewOfFile(hMapping);

    puts("");
    return EXIT_SUCCESS;
}
