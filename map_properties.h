#define COUNT_PROCESSES 4
#define BUF_SIZE 7
#define MESSAGE_LENGTH 10
#define MAP_NAME "Super unique map name (wow!)"

int min(int x, int y)
{
  if (x < y)
    return x;
  else
    return y;
}

#include <Windows.h>

typedef struct {
  int cur_index;
  int max_index;
  int buffer[BUF_SIZE];
  size_t written_to_buffer;
} SharedMemory;
