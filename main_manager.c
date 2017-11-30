// Основная программа, должна быть запущена первой. Вызывает COUNT_PROCESSES экземпляров программы writer,
// которая получает порядковый номер и последовательно записывает его MESSAGE_LENGTH раз в буфер в общей памяти,
// откуда это сообщение должно быть забрано запущенной после данной программы программой reader и выведено
// ею на консоль.

#include "map_properties.h"

#include <stdio.h>
#include <Windows.h>
#include <tchar.h>
#include <string.h>

int main()
{
    HANDLE hMapping = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, BUF_SIZE, _T(MAP_NAME));
	if (!hMapping) {
        printf("Manager: Can't create file mapping! Error code:%d\n", GetLastError());
        return EXIT_FAILURE;
    }

	SharedMemory * sh_data = (SharedMemory*) MapViewOfFile(hMapping, FILE_MAP_WRITE, 0, 0, 0);
	if (!sh_data) {
		printf("Manager: Can't map view of file! Error code:%d\n", GetLastError());
        return EXIT_FAILURE;
    }

	sh_data->cur_index = 0;
	sh_data->max_index = COUNT_PROCESSES - 1;
	sh_data->written_to_buffer = 0;

	HANDLE can_read = CreateEvent(NULL, TRUE, FALSE, _T("CAN_READ_EVENT"));
	HANDLE can_write = CreateEvent(NULL, TRUE, TRUE, _T("CAN_WRITE_EVENT"));
	HANDLE mutex = CreateMutex(NULL, FALSE, _T("MUTEX"));

	PROCESS_INFORMATION pi_arr[COUNT_PROCESSES];
	STARTUPINFO si_arr[COUNT_PROCESSES];

	PROCESS_INFORMATION pi = {
		INVALID_HANDLE_VALUE,
		INVALID_HANDLE_VALUE,
		0,
		0
	};

	STARTUPINFO si = {
		sizeof(STARTUPINFO),
		0
	};

	for (size_t i = 0; i < COUNT_PROCESSES; ++i) {
        ZeroMemory(&pi_arr[i], sizeof(pi_arr[i]));
        ZeroMemory(&si_arr[i], sizeof(si_arr[i]));
        si_arr[i].cb = sizeof(&si_arr[i]);

		const char * process_title = "writer";
		char cmd_line[strlen(process_title) + 5];
		sprintf(cmd_line, "%s %d\0", process_title, i);

		BOOL result = CreateProcess(NULL, _T(cmd_line), NULL, NULL, FALSE, 0, NULL, NULL, &si_arr[i], &pi_arr[i]);
		if (!result) {
			printf("Error when create process! Error code:%d\n", GetLastError());
			return EXIT_FAILURE;
		}
	}

	for (size_t i = 0; i < COUNT_PROCESSES; ++i)
	{
		WaitForSingleObject(pi_arr[i].hProcess, INFINITE);
		CloseHandle(pi_arr[i].hProcess);
		CloseHandle(pi_arr[i].hThread);
	}

    UnmapViewOfFile(hMapping);
	return EXIT_SUCCESS;
}
